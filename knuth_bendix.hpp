#ifndef KNUTH_BENDIX_HPP
#define KNUTH_BENDIX_HPP

#include <cassert>
#include <list>
#include <deque>
#include "aho_corasick.hpp" // fast multi-string pattern search
#include "murmur3.h"
#include <memory>
#include <set>

struct RuntimeLimiter;

//#define debugline(x) std::cerr << x << std::endl;
#ifndef debugline
#define debugline(x) ;

#endif

template<typename iteratortype>
std::string toString(const iteratortype &b, const iteratortype &e) noexcept {
    std::ostringstream oss;
    for (auto i = b; i != e; ++i) {
        oss << *i << " ";
    }
    return oss.str();
}

template<typename containertype>
std::string toString(const containertype &c) noexcept {
    return toString(c.begin(), c.end());
}

/*
// This implementation of lower_bound doesnt require a reference to the value.
// But it breaks the convention that a comparator accepts 2 params.
template<typename _ForwardIterator, typename _Compare>
_ForwardIterator lower_bound2(_ForwardIterator __first,
                              const _ForwardIterator &__last,
                              const _Compare &__comp) {
    auto __len = std::distance(__first, __last);
    while (__len > 0) {
        auto __half = __len >> 1;
        _ForwardIterator __middle = __first;
        std::advance(__middle, __half);
        if (__comp(*__middle)) {
            __first = __middle;
            ++__first;
            __len = __len - __half - 1;
        } else
            __len = __half;
    }
    return __first;
}

// This allows for using a vector as lookup while storing the payload elsewhere.
template<class vector_type, typename constructortype, typename eqtest, typename comparatortype>
typename vector_type::const_iterator insertOrderedUnique(vector_type &v,
                                                         const constructortype &construct, //std::function<typename vector_type::value_type()>
                                                         const eqtest &eq,
                                                         const comparatortype &comparator) { //std::function<bool(typename vector_type::value_type())>&
    auto it = lower_bound2(v.begin(),
                           v.end(),
                           comparator); // find proper position in descending order
    if (it == v.end() || !eq(*it)) {
        return v.insert(it, construct());
    }
    return it;
}
*/
template<typename stringtype, typename indextype>
struct StringStorage {
    typedef typename stringtype::value_type symboltype;
    //   typedef typename symbolinfo::stringtype stringtype;
    std::deque<stringtype> strings; // all strings get stored here.
    std::deque<indextype> ordered_stringindexes; // ordered by their associated string. this is the lookup-map for strings.
public:
    std::function<bool(const symboltype &, const symboltype &)> comp = [](const symboltype &a, const symboltype &b) { return a < b; };
    std::function<bool(const symboltype &, const symboltype &)> eq = [](const symboltype &a, const symboltype &b) { return a == b; };

    template<typename iteratortype>
    indextype getOrCreateString(const iteratortype &begin, const iteratortype &end) noexcept {


        // check ordered_stringindexes if it exists already, if not store it in strings and put the index in ordered_stringindexes. return the index.
        indextype ret = *insertOrderedUnique(ordered_stringindexes,
                                             [&]() { // constructor
                                                 indextype ret_ = strings.size();
                                                 strings.push_back(stringtype(begin, end));
//                                                 assert(std::equal(begin, end, strings[ret_].begin(), strings[ret_].end(), eq));
                                                 return ret_;
                                             },
                                             [&](const indextype &index) -> bool {
                                                 const auto &o = strings[index];
                                                 return std::equal(begin, end, o.begin(), o.end(), eq); // we might want a binary predicate from symbolinfo...
                                             },
                                             [&](const indextype &index) -> bool { // comparator
                                                 const auto &o = strings[index];
                                                 //return kbc.complexity_comparison(o.begin(), o.end(), begin,end);
                                                 return std::lexicographical_compare(o.begin(), o.end(), begin, end, comp);
                                             });

        debugline("string index " << ret << " is " << toString(begin, end));
        return ret;
    }

    template<typename stringtype2>
    indextype getOrCreateString(const stringtype2 &s) noexcept {
        return getOrCreateString(s.begin(), s.end());
    }

    const stringtype &get(const indextype i) const noexcept {
        return strings[i];
    }
};

template<typename stringtype, typename stringid>
struct KnuthBendixCompletion {
    typedef typename stringtype::value_type symboltype;
    typedef StringStorage<stringtype, stringid> stringstoragetype;
    stringstoragetype *ss; // the stringstorage is not owned by this.

    std::vector<stringid> eqclasses; // linked lists, circular, easily merged, easily iterated.

    std::deque<std::function<bool(const stringtype &, const stringtype &, RuntimeLimiter* )> > complexity_orderings;
    std::deque<std::vector<bool> > is_elite; // for each complexity ordering


    std::set<std::pair<stringid, stringid> > checked_for_expansion;
    std::set<std::pair<stringid, stringid> > checked_for_deducing;

    aho_corasick::basic_trie<symboltype, std::vector<stringid> > actree;// given a inputstring this will efficiently give all the matched strings
    //aho_corasick::basic_trie<symboltype, int > minimum_costs; // infinite cases ... to be avoided ...

    std::deque<std::vector<bool>> makes_sense_for_given_order; // in many cases we might detect a match but should do nothing with it, it would only be redundant
    std::size_t max_allowed_string_size = std::numeric_limits<std::size_t>::max();
    bool clean;

    KnuthBendixCompletion(stringstoragetype *ss_) noexcept:
            ss(ss_),
            clean(false) {
        // shortlex is of interest anyway. At least one decent order is needed for deducing.
        addOrdering([&](const stringtype &a, const stringtype &b, RuntimeLimiter* ) {
            // shortlex ordering.
            if (a.size() < b.size())
                return true;
            if (a.size() > b.size())
                return false;
            return std::lexicographical_compare(a.begin(), a.end(), b.begin(), b.end(), ss->comp);
        });
    }

    struct Bits128 {
        uint64_t a, b;

        bool operator<(const Bits128 &o) const {

            if (o.a == a) {
                return o.b < b;
            } else {
                return o.a < a;
            }
        }
    };

    std::size_t addOrdering(const std::function<bool(const stringtype &, const stringtype &, RuntimeLimiter* )> &order) noexcept {

        // TODO: ... all those
        complexity_orderings.push_back(order);
        std::vector<bool> &e = is_elite.emplace_back(eqclasses.size(), true);
        std::size_t orderindex = is_elite.size() - 1;
        makes_sense_for_given_order.emplace_back(eqclasses.size(), true);

        // filter out the elite.
        iterateEqClasses([&](const stringid representativeid) {
            assert(e[representativeid]); // this must be because we just initialised it that way and this is supposed to be an untouched eqclass
            iterateEqClassElite(representativeid, [&](const stringid s1) {
                iterateEqClassElite(representativeid, [&](const stringid s2) {
                    if (s1 != s2 && complexity_orderings[orderindex](ss->get(s1), ss->get(s2),0)) {
                        is_elite[orderindex][s2] = false;
                        std::cerr << '^';
                    }
                    return true;
                }, orderindex);
                return true;
            }, orderindex);
            return true;
        });
        clean = false;
        return orderindex;
    }

    template<typename iteratortype>
    stringid getOrCreateString(const iteratortype &begin, const iteratortype &end) noexcept {
        auto ret = ss->getOrCreateString(begin, end);
        if (eqclasses.size() <= ret) {
            if (begin != end) {
                actree.getOrCreate(begin, end).push_back(ret);
            }
            while (eqclasses.size() <= ret) {
                eqclasses.push_back(eqclasses.size());
            }


            for (auto &ie : is_elite) {
                while (ie.size() < eqclasses.size()) {
                    ie.push_back(true);
                }
            }
            for (auto &ie : makes_sense_for_given_order) {
                while (ie.size() < eqclasses.size()) {
                    ie.push_back(true);
                }
            }
        }
        return ret;
    }

    template<typename stringtype2>
    stringid getOrCreateString(const stringtype2 &s) noexcept {
        return getOrCreateString(s.begin(), s.end());
    }

    // overriding this (using eg partial specialisation) can be very handy.


    void addIdentity(const stringtype &a, const stringtype &b, RuntimeLimiter *runtimelimiter) noexcept {
        if (addIdentity(getOrCreateString(a), getOrCreateString(b), runtimelimiter)) {
            clean = false;
        }
    }

// this function separated from expand() to allow specializing it further.
    bool expand_getAllRewrites(const stringid i,RuntimeLimiter* runtimelimiter) noexcept {
        bool changed_something = false;
        std::set<std::pair<stringid, stringid> > checked_extra;
        const auto &s = ss->get(i);
        actree.iterate_matches(s.begin(),
                               s.end(),
                               [&](const std::vector<stringid> &matches,
                                   const typename stringtype::iterator &posbegin,
                                   const typename stringtype::iterator &posend) {
                                   if (s.begin() == posbegin && s.end() == posend) {
                                       return true; // the entire thing matched, doing replacements here would give no new info.
                                   }
                                   for (const stringid match : matches) {
                                       if (!makes_sense_for_given_order[0][match] || eqclasses[match] == match || checked_for_expansion.find({i, match}) != checked_for_expansion.end()) {
                                           continue;
                                       }
                                       checked_extra.insert({i, match});
                                       iterateEqClass/*Elite*/(match, [&](const stringid replacementid) {
                                           if (match == replacementid) {
                                               return;
                                           }
                                           stringtype s2(s.begin(), posbegin);
                                           const stringtype &replacement = ss->get(replacementid);
                                           s2.insert(s2.end(), replacement.begin(), replacement.end());
                                           s2.insert(s2.end(), posend, s.end());
                                           if (addIdentity(i, getOrCreateString(s2))) {
                                               changed_something = true;
                                           }
                                       });
                                   }
                                   return true; // keep iterating... i want them all...
                               });
        checked_for_expansion.insert(checked_extra.begin(), checked_extra.end());
        return changed_something;
    }


    void expand(RuntimeLimiter* runtimelimiter) noexcept {
        if (clean) {
            return;
        }
        std::cerr << "EXPAND";

        bool changed_something = false;
        for (stringid i = 0; i < eqclasses.size(); i++) {
            if (eqclasses[i] == i) {
                continue;
            }
            if (expand_getAllRewrites(i,runtimelimiter)) {
                changed_something = true;
            }
        }

        // try to discover new identities through knuth-bendix deduce
        for (stringid i = 0; i < eqclasses.size(); i++) {
            if (eqclasses[i] == i) {
                continue;
            }
            const stringtype large = ss->get(i);
            const int s1size = large.size();

            for (stringid j = 0; j < eqclasses.size(); j++) {
                if (eqclasses[j] == j) {
                    continue;
                }
                if (checked_for_deducing.find(std::make_pair(i, j)) != checked_for_deducing.end()) {
                    continue;
                }
                checked_for_deducing.insert(std::make_pair(i, j));
                const stringtype small = ss->get(i);
                const int s2size = small.size();
                if (large.size() < small.size() || large.size() == 0 || small.size() == 0) {
                    continue;
                }

                for (int offset = 1 - s2size; offset < s1size; ++offset) {
                    if (offset >= 0 && offset <= s1size - s2size) {
                        offset = 1 + s1size - s2size; // TODO: check this, might not be enough.
                    }

//                    const auto &s1 = ss->get(large.first); // these need to be within the loop because the content of strings gets modified during the iterations. Though ... with a deque i dont expect much issues.
//                    const auto &s2 = ss->get(small.first);
                    int l = small.size();
                    if (offset < 0) {
                        l += offset;
                    }
                    if (offset + small.size() > large.size()) {
                        l -= (offset + (int) small.size() - (int) large.size());
                    }

                    if (((std::size_t) ((int) s1size) + ((int) s2size) - l) > max_allowed_string_size) {
                        continue;
                    }
                    if (l < 1) {
                        continue;
                    }
                    //prt2(large.first, strings.size());
                    //prt6(offset, l, s1.size(), s2.size(), toString(s1),toString(s2));
                    assert(l);
                    const auto large_overlapbegin = large.begin() + std::max(0, offset);
                    const auto large_overlapend = large_overlapbegin + l;
                    const auto small_overlapbegin = small.begin() + std::max(0, -offset);
                    const auto small_overlapend = small_overlapbegin + l;
                    if (std::equal(large_overlapbegin,
                                   large_overlapend,
                                   small_overlapbegin,
                                   small_overlapend,
                                   ss->eq)) [[unlikely]] {

                        // equality new_identity; // == a critical pair
                        iterateEqClassElite(i, [&](const stringid large_replacement_id) {
                            const stringtype &large_replacement = ss->get(large_replacement_id);
                            iterateEqClassElite(j, [&](const stringid small_replacement_id) {
                                const stringtype &small_replacement = ss->get(small_replacement_id);

                                if (offset < 0) {
                                    stringtype cp1(small.begin(), small_overlapbegin);
                                    cp1.insert(cp1.end(), large_replacement.begin(), large_replacement.end());
                                    stringtype cp2(small_replacement);
                                    cp2.insert(cp2.end(), large_overlapend, large.end());
                                    if (addIdentity(getOrCreateString(cp1), getOrCreateString(cp2),runtimelimiter)) {
                                        changed_something = true;
                                    }


                                } else if (offset + s2size > s1size) {

                                    stringtype cp1(large.begin(), large_overlapbegin);
                                    cp1.insert(cp1.end(), small_replacement.begin(), small_replacement.end());
                                    stringtype cp2(large_replacement);
                                    cp2.insert(cp2.end(), small_overlapend, small.end());

                                    if (addIdentity(getOrCreateString(cp1), getOrCreateString(cp2),runtimelimiter)) {
                                        changed_something = true;
                                    }

                                } else { // is this useful ? would this replacement not have been done during expand already...?
                                    assert(false); // try to skip this step.
                                    stringtype cp1(large.begin(), large.begin() + std::max(0, offset));
                                    cp1.insert(cp1.end(), small_replacement.begin(), small_replacement.end());
                                    cp1.insert(cp1.end(), large.begin() + std::max(0, offset) + l, large.end());

                                    if (addIdentity(large_replacement_id, getOrCreateString(cp1),runtimelimiter)) {
                                        changed_something = true;
                                    }
                                }
                                return true;

                            });
                            return true;
                        });


                    }
                }

            }
        }
        if (changed_something) [[likely]] {
            expand(runtimelimiter);
        } else {
            for (std::size_t orderindex = 0; orderindex < complexity_orderings.size(); ++orderindex) {
                collapse(orderindex);
            }
            clean = true;
        }
        std::cerr << " EXPAND ENDED";
    }

    void print() const noexcept {};


    std::vector<stringtype> attemptFullSimulation(const stringtype &s,
                                                  const std::size_t order,
                                                  RuntimeLimiter* runtimelimiter) noexcept {
        assert(false); // TODO...
    }

    // ... todo... some orderings ... should be applied/checked AFTER the rewrite !
    void collapse(std::size_t orderindex) { // == collapse, per order and in all possible ways

        for (stringid patientid = 0; patientid < this->eqclasses.size(); ++patientid) {
            auto &is_elite2 = is_elite[orderindex];
            if (is_elite2[patientid]) {
                continue; // nothing to rewrite...
            }
            if (!makes_sense_for_given_order[orderindex][patientid]) {
                continue; // it is already out of the way.
            }
            bool original_can_be_rewritten_with_other_rules_thus_is_not_needed_anymore = false;
            const auto &patient = ss->get(patientid);
            actree.iterate_matches(patient.begin(), patient.end(), [&](const std::vector<stringid> &matches,
                                                                       const typename stringtype::iterator &posbegin,
                                                                       const typename stringtype::iterator &posend) {
                for (const auto m : matches) {
                    if (m != patientid && !is_elite[orderindex][m]) {
                        // "when ignoring the rule with i as the to-be-replaced-part ... there still are other rules getting triggered ... "
                        original_can_be_rewritten_with_other_rules_thus_is_not_needed_anymore = true;
                        return false;
                    }
                }
                return true;
            });
            if (original_can_be_rewritten_with_other_rules_thus_is_not_needed_anymore) {
                makes_sense_for_given_order[orderindex][patient] = false;
            }
        }
    }


    //TODO: put the logic for these eqclasses in utils, it keeps coming back (see mrss-linked-lists), it is useful.
    bool addIdentity(const stringid part_index_a, const stringid part_index_b, RuntimeLimiter* runtimelimiter) noexcept {
        //assertIdentity({part_index_a, part_index_b});

        auto prepareAdditionalStringStuff = [&](const stringid si) {
            if (eqclasses.size() <= si) {
                const auto &s = ss->get(si);
                if (!s.empty()) {
                    actree.getOrCreate(s).push_back(si);
                }
                while (eqclasses.size() <= si) {
                    eqclasses.push_back(0);
                    for (auto &ie : is_elite) {
                        ie.push_back(true); // this thing will be its own elite...
                    }
                }
            }
        };
        prepareAdditionalStringStuff(part_index_a);
        prepareAdditionalStringStuff(part_index_b);

        assert(part_index_a < eqclasses.size());
        assert(part_index_b < eqclasses.size());
        // each of these belongs to a ordered single linked list .. which loops (the lowest index points to the beginning)



        // now ... combine the 2 of them!

        // got to iterate the both of them once ... simultaneously .. zipping them on the go ...
        // got to find the start first ...

        std::size_t a_i = part_index_a;
        std::size_t b_i = part_index_b;

        while (eqclasses[a_i] > a_i) {
            a_i = eqclasses[a_i];
        }

        while (eqclasses[b_i] > b_i) {
            b_i = eqclasses[b_i];
        }

        std::size_t a_begin = eqclasses[a_i];
        std::size_t b_begin = eqclasses[b_i];
        if (a_begin == b_begin) {
            // they are linked already!
            return false;
        }

        for (std::size_t order = 0; order < complexity_orderings.size(); ++order) { // update the elite, easiest done before the merge.
            iterateEqClassElite(part_index_a,
                                [&](const stringid elite_a) -> bool {
                                    iterateEqClassElite(part_index_b,
                                                        [&](const stringid elite_b) -> bool {
                                                            if (elite_a != elite_b && complexity_orderings[order](ss->get(elite_a), ss->get(elite_b), runtimelimiter)) {
                                                                //assert(false); // please fail
                                                                is_elite[order][elite_b] = false;
                                                            }
                                                            return true;
                                                        }, order);
                                    return true;
                                }, order);
        }


        std::size_t combined_begin;
        std::size_t combined_i;
        auto &eqclasses = this->eqclasses;
        auto setReturnRef = [&combined_i, &eqclasses, &combined_begin]() {
            while (combined_i < eqclasses[combined_i]) {
                combined_i = eqclasses[combined_i];
            }
            eqclasses[combined_i] = combined_begin;
        };

        if (a_begin < b_begin) {
            combined_begin = a_begin;
            combined_i = combined_begin;
            a_i = eqclasses[a_begin];
            b_i = b_begin;
            if (a_i == a_begin) {
                eqclasses[combined_begin] = b_i;
                setReturnRef();
                return true;
            }
        } else {
            combined_begin = b_begin;
            combined_i = combined_begin;
            a_i = a_begin;
            b_i = eqclasses[b_begin];
            if (b_i == b_begin) {
                eqclasses[combined_begin] = a_i;
                setReturnRef();
                return true;
            }
        }


// somehow at the end of a previous merge the other_mrss_member_partindex was not set to combined_begin
        while (true) {
            if (a_i < b_i) {
                eqclasses[combined_i] = a_i;
                combined_i = a_i;

                a_i = eqclasses[a_i];

                if (a_i == a_begin) {
                    eqclasses[combined_i] = b_i;
                    //return;
                    break;
                }
            } else {
                eqclasses[combined_i] = b_i;
                combined_i = b_i;
                b_i = eqclasses[b_i];

                if (b_i == b_begin) {
                    eqclasses[combined_i] = a_i;
                    //return;
                    break;
                }
            }
        }
        setReturnRef();
        return true;
    }

    template<typename callbackfcttype>
    void iterateEqClass(const stringid part, const callbackfcttype &cb) const noexcept {
        cb(part);
        if (eqclasses[part] == part) {
            return;
        }
        for (stringid i = eqclasses[part];
             i != part;
             i = eqclasses[i]) {
            //prt3(i, eqclasses[i], part);
            if (!cb(i)) {
                return;
            }
        }
    }

    template<typename callbackfcttype>
    void iterateEqClassElite(stringid part, const callbackfcttype &cb, const std::size_t order = 0) const noexcept {
        iterateEqClass(part, [&](stringid part_) -> bool {
            if (is_elite[order][part_]) {
                if (!cb(part_)) {
                    return false;
                }
            }
            return true;
        });
    }

    template<typename callbackfcttype>
    void iterateEqClasses(const callbackfcttype &cb) const noexcept {
        for (std::size_t i = 0; i < eqclasses.size(); ++i) {
            // trying to keep just 1 representative per eqclass here: the tail (?).
            if (eqclasses[i] > i) {
                continue;
            }
            if (!cb(i)) {
                return;
            }
        }
    }

};


#endif

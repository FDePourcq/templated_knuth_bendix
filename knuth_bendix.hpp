#ifndef KNUTH_BENDIX_HPP
#define KNUTH_BENDIX_HPP

#include <cassert>
#include <list>
#include <deque>
#include "aho_corasick.hpp" // fast multi-string pattern search
#include "murmur3.h"
#include <memory>
#include <set>

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
class StringStorage {
    typedef typename stringtype::value_type symboltype;
    //   typedef typename symbolinfo::stringtype stringtype;
    std::deque<stringtype> strings; // all strings get stored here.
    std::deque<indextype> ordered_stringindexes; // ordered by their associated string. this is the lookup-map for strings.
public:
    std::function<bool(const symboltype &, const symboltype &)> comp = [](const symboltype &a, const symboltype &b) { return a < b; };
    std::function<bool(const symboltype &, const symboltype &)> eq = [](const symboltype &a, const symboltype &b) { return a == b; };

    template<typename iteratortype>
    indextype getOrCreateString(const iteratortype &begin, const iteratortype &end) noexcept {
//if (false){
//        for (auto i = begin; i != end; ++i) {
//            assert((*i).numcalls == 1 || (*i).numcalls == -1);
//        }
//
//        for (std::size_t i = 0; i < strings.size(); ++i) {
//            for (std::size_t j = i + 1; j < strings.size(); ++j) {
//                if (std::equal(strings[j].begin(), strings[j].end(), strings[i].begin(), strings[i].end(), eq)) {
//                    std::cerr << "duplicate(3)!!! " << i << "\t\t" << toString(strings[i]) << std::endl;
//                    std::cerr << "duplicate(3)!!! " << j << "\t\t" << toString(strings[j]) << std::endl;
//                    for (auto ii : ordered_stringindexes) {
//                        std::cerr << "  index: " << ii << "\t: " << toString(strings[ii]) << " size: " << strings[ii].size() << std::endl;
//                    }
//                    assert(false);
//                }
//            }
//        }
//}

        // check ordered_stringindexes if it exists already, if not store it in strings and put the index in ordered_stringindexes. return the index.
        indextype ret = *insertOrderedUnique(ordered_stringindexes,
                                             [&]() { // constructor


//                                                 std::cerr << " adding " << toString(begin, end) << std::endl;
//                                                 for (std::size_t i = 0; i < strings.size(); ++i) {
//                                                     if (std::equal(begin, end, strings[i].begin(), strings[i].end(), eq)) {
//                                                         std::cerr << "when adding " << toString(begin, end) << "   see index " << i << std::endl;
//                                                         for (auto ii : ordered_stringindexes) {
//                                                             std::cerr << "  index: " << ii << "\t: " << toString(strings[ii]) << " size: " << strings[ii].size() << std::endl;
//                                                         }
//                                                         assert(false);
//                                                     }
//                                                     for (std::size_t j = i + 1; j < strings.size(); ++j) {
//                                                         if (std::equal(strings[j].begin(), strings[j].end(), strings[i].begin(), strings[i].end(), eq)) {
//                                                             std::cerr << "duplicate!!! " << i << "\t\t" << toString(strings[i]) << std::endl;
//                                                             std::cerr << "duplicate!!! " << j << "\t\t" << toString(strings[j]) << std::endl;
//                                                             for (auto ii : ordered_stringindexes) {
//                                                                 std::cerr << "  index: " << ii << "\t: " << toString(strings[ii]) << " size: " << strings[ii].size() << std::endl;
//                                                             }
//                                                             assert(false);
//                                                         }
//                                                     }
//                                                 }
                                                 indextype ret_ = strings.size();
                                                 strings.push_back(stringtype(begin, end));
//                                                 assert(std::equal(begin, end, strings[ret_].begin(), strings[ret_].end(), eq));

//                                                 for (std::size_t i = 0; i < strings.size(); ++i) {
//                                                     for (std::size_t j = i + 1; j < strings.size(); ++j) {
//                                                         if (std::equal(strings[j].begin(), strings[j].end(), strings[i].begin(), strings[i].end(), eq)) {
//                                                             std::cerr << "duplicate(4)!!! " << i << "\t\t" << toString(strings[i]) << std::endl;
//                                                             std::cerr << "duplicate(4)!!! " << j << "\t\t" << toString(strings[j]) << std::endl;
//                                                             for (auto ii : ordered_stringindexes) {
//                                                                 std::cerr << "  index: " << ii << "\t: " << toString(strings[ii]) << std::endl;
//                                                             }
//                                                             assert(false);
//                                                         }
//                                                     }
//                                                 }

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
        for (std::size_t i = 0; i < strings.size(); ++i) {
//            if (std::equal(begin, end, strings[i].begin(), strings[i].end(), eq)) {
//                std::cerr << "getOrCreateString " << toString(begin, end) << std::endl;
//                for (auto ii : ordered_stringindexes) {
//                    std::cerr << "  index: " << ii << "\t: " << toString(strings[ii]) << std::endl;
//                }
//                assert(false);
//            }
//#ifdef DEBUGGING
//            for (std::size_t j = i + 1; j < strings.size(); ++j) {
//                if (std::equal(strings[j].begin(), strings[j].end(), strings[i].begin(), strings[i].end(), eq)) {
//                    debugline("duplicate(2)!!! " << i << "\t\t" << toString(strings[i]) << '\n');
//                    debugline("duplicate(2)!!! " << j << "\t\t" << toString(strings[j]) << '\n');
//                    for (auto ii : ordered_stringindexes) {
//                        std::cerr << "  index: " << ii << "\t: " << toString(strings[ii]) << " size: " << strings[ii].size() << std::endl;
//                    }
//                    assert(false);
//                }
//            }
//#endif // DEBUGGING
        }debugline("string index " << ret << " is " << toString(begin, end));
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

    typedef std::pair<stringid, stringid> equality; // == identity
    typedef std::pair<stringid, stringid> rule;
    typedef std::set<equality> identities;
    typedef std::set<rule> rules;

    identities input_identities; // the inputs are stored to allow reprocessing them.
    identities current_identities; // these will be converted into rules eventually
    rules current_rules; // these are going to be rewritten till they converge ... if they converge at all ...
    aho_corasick::basic_trie<stringtype, rules> actree; // given a inputstring this will efficiently give all the rules that got triggered (and where...).
    bool got_confluent_rewrite_system;
    std::size_t max_allowed_string_size = std::numeric_limits<std::size_t>::max();

    bool converged; // set when we reached a previous state, meaning that any more runs wont have any effect. (unless new info is added)

    KnuthBendixCompletion(stringstoragetype *ss_) noexcept:
            ss(ss_),
            got_confluent_rewrite_system(false),
            converged(false){
    }

    // override this with a custom ordering.
    std::function<bool(const stringtype &, const stringtype &)> complexity_comparison = [&](const stringtype &a, const stringtype &b) {
        // shortlex ordering.
        if (a.size() < b.size())
            return true;
        if (a.size() > b.size())
            return false;
        return std::lexicographical_compare(a.begin(), a.end(), b.begin(), b.end(), ss->comp);
    };

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


    std::set<Bits128> hashed_states; // anti-loop detection

    template<typename iteratortype>
    stringid getOrCreateString(const iteratortype &begin, const iteratortype &end) noexcept {
        return ss->getOrCreateString(begin, end);
    }

    template<typename stringtype2>
    stringid getOrCreateString(const stringtype2 &s) noexcept {
        return ss->getOrCreateString(s.begin(), s.end());
    }

    // overriding this (using eg partial specialisation) can be very handy.
    equality orderIdentity(const equality &eq) noexcept {
        if (complexity_comparison(ss->get(eq.first), ss->get(eq.second))) {
            return equality(eq.second, eq.first);
        }
        return eq;
    }


    void addIdentity(const stringtype &a, const stringtype &b) noexcept {
        got_confluent_rewrite_system = false;
        converged = false;
        const auto identity = orderIdentity(std::make_pair(getOrCreateString(a), getOrCreateString(b)));
        input_identities.insert(identity);
    }

    void addIdentity(const stringid a, const stringid b) noexcept {
        got_confluent_rewrite_system = false;
        converged = false;
        const auto identity = orderIdentity(std::make_pair(a, b));
        input_identities.insert(identity);
    }

    void tryDelete() noexcept {
        for (auto i = current_identities.begin(); i != current_identities.end();) {
            const auto &s1 = ss->get(i->first);
            const auto &s2 = ss->get(i->second);
            if (std::equal(s1.begin(), s1.end(), s2.begin(), s2.end(), ss->eq)) {
                i = current_identities.erase(i);
            } else {
                ++i;
            }
        }
    }

//    template<typename iteratortype, typename callbacktype, typename stranditeratortype>
//    void reduceWithMultipleStrands(const iteratortype &begin, const iteratortype &end, const callbacktype &fct) {
//        std::list<symboltype> ret(begin, end);
//        bool changed = false;
//        bool changed_local = false;
//        //std::cerr << "[";
//
//        do {
//            auto lvl1begin = stranditeratortype(ret.begin());
//            auto lvl1end = stranditeratortype(ret.end());
//            // i need a wrapper for the list ... so that it produces level-aware iterators ...
//            changed_local = false;
//            for (auto i = lvl1begin; !changed_local && i != lvl1end; ++i) {
//
//                actree.iterate_matches(i->begin(),
//                                       i->end(),
//                                       [&](const rules &r,
//                                           const typename std::list<symboltype>::iterator &posbegin,
//                                           const typename std::list<symboltype>::iterator &posend) {
//                                           if (r.empty()) {
//                                               return true;
//                                           }
//                                           const auto &replacement = ss->get(r.begin()->second);
//                                           //std::cerr << " from string " << std::string(ret.begin(), ret.end()) << " , particularly " << std::string(posbegin,posend) << "  can be replaced by " << std::string(replacement.begin(), replacement.end())  << std::endl;
//
//                                           //std::cerr << " match is " << toString(posbegin,posend) <<std::endl;
//                                           //std::cerr << "applying rule " << toString(get(r.begin()->first)) << " --> " << toString(get(r.begin()->second)) << " : " << toString(ret);
//                                           const auto begin2 = i->erase(posbegin, posend);
//                                           i->insert(begin2, replacement.begin(), replacement.end());
//                                           //std::cerr << " ==> " << toString(ret) << std::endl;
//
//                                           changed_local = true;
//                                           changed = true;
//                                           return false;
//                                       });
//            }
//
//        } while (changed_local);
//        //std::cerr << "]";
//        fct(changed, ret.begin(), ret.end());
//    }

    template<typename iteratortype, typename callbacktype>
    void reduce(const iteratortype &begin, const iteratortype &end, const callbacktype &fct) noexcept {
        std::list<symboltype> ret(begin, end);
        bool changed = false;
        bool changed_local = false;
        //std::cerr << "[";
        // i must ... introduce loopdetection ...
        do {
            // i need a wrapper for the list ... so that it produces level-aware iterators ...
            changed_local = false;
            actree.iterate_matches(ret.begin(),
                                   ret.end(),
                                   [&](const rules &r,
                                       const typename std::list<symboltype>::iterator &posbegin,
                                       const typename std::list<symboltype>::iterator &posend) {
                                       if (r.empty()) {
                                           return true;
                                       }
                                       const auto &replacement = ss->get(r.begin()->second);
                                       //std::cerr << " from string " << std::string(ret.begin(), ret.end()) << " , particularly " << std::string(posbegin,posend) << "  can be replaced by " << std::string(replacement.begin(), replacement.end())  << std::endl;

                                       //std::cerr << " match is " << toString(posbegin,posend) <<std::endl;
                                       //std::cerr << "applying rule " << toString(get(r.begin()->first)) << " --> " << toString(get(r.begin()->second)) << " : " << toString(ret);
                                       const auto begin2 = ret.erase(posbegin, posend);
                                       ret.insert(begin2, replacement.begin(), replacement.end());
                                       //std::cerr << " ==> " << toString(ret) << std::endl;

                                       changed_local = true;
                                       changed = true;
                                       return false;
                                   });

        } while (changed_local);
        //std::cerr << "]";
        fct(changed, ret.begin(), ret.end());
    }

    template<typename stringtype2, typename callbacktype>
    void reduce(const stringtype2 &s, const callbacktype &fct) noexcept {
        reduce(s.begin(), s.end(), fct);
    }


    // what about using std::optional as returntype?
    template<typename iteratortype>
    std::pair<std::list<symboltype>, bool> reduceCopy(const iteratortype &begin, const iteratortype &end) noexcept {
        std::pair<std::list<symboltype>, bool> ret(std::list<symboltype>(begin, end), false);
        bool changed_local = false;
        do {
            changed_local = false;
            actree.iterate_matches(ret.first.begin(),
                                   ret.first.end(),
                                   [&](const rules &r,
                                       const typename std::list<symboltype>::iterator &posbegin,
                                       const typename std::list<symboltype>::iterator &posend) {
                                       if (r.empty()) {
                                           return true;
                                       }
                                       const auto &replacement = ss->get(r.begin()->second);
                                       //std::cerr << " match is " << toString(posbegin,posend) <<std::endl;
                                       //std::cerr << "applying rule " << toString(get(r.begin()->first)) << " --> " << toString(get(r.begin()->second)) << " : " << toString(ret);
                                       const auto begin2 = ret.first.erase(posbegin, posend);
                                       ret.first.insert(begin2, replacement.begin(), replacement.end());
                                       //std::cerr << " ==> " << toString(ret) << std::endl;

                                       changed_local = true;
                                       ret.second = true;
                                       return false;
                                   });
        } while (changed_local);
        return ret;
    }

    std::pair<std::list<symboltype>, bool> reduceCopy(const stringtype &s) noexcept {
        return reduceCopy(s.begin(), s.end());
    }


    template<typename iteratortype>
    stringid reduceCopyRegister(const iteratortype &begin, const iteratortype &end) noexcept {
        stringid ret;
        reduce(begin,
               end,
               [&](const bool,//changed,
                   const typename std::list<symboltype>::iterator &begin,
                   const typename std::list<symboltype>::iterator &end) {
                   ret = getOrCreateString(begin, end);
               });
        return ret;
    }

    template<typename stringtype2>
    stringid reduceCopyRegister(const stringtype2 &s) noexcept {
        return reduceCopyRegister(s.begin(), s.end());
    }

    void assertIdentity(const std::pair<stringid, stringid> &/*r*/) const noexcept {
        // can be overriden by the user.
    }

    void tryCompose() noexcept {
        // this only modifies rules, but the actree needs to be updated in tandem.
        for (auto i = current_rules.begin(); i != current_rules.end();) {
            const auto &s1 = ss->get(i->first);
            const auto &s2 = ss->get(i->second);
            auto mmm = reduceCopy(s2.begin(), s2.end());
            if (mmm.second && mmm.first.size() < max_allowed_string_size) { debugline("tryCompose: rewriting " << toString(s2) << " to " << toString(mmm.first));
                //assert(sum(s2) == sum(mmm.first));
                //assertss(sum(s2) == sum(mmm.first), pt(sum(s2)) << pt(sum(mmm.first)) );
                auto *actreenode = actree.getNodeOrCreate(s1);
                assert(actreenode->payload.has_value());
                actreenode->payload->erase(*i);
                rule new_rule = std::make_pair(i->first, getOrCreateString(mmm.first.begin(), mmm.first.end()));
                //assert(new_rule.second != i->second); // we are supposed to have changed something remember...
                i = current_rules.erase(i);
                assertIdentity(new_rule);
                if (new_rule.first != new_rule.second) { debugline("tryCompose: adding rule " << toString(ss->get(new_rule.first)) << " to " << toString(ss->get(new_rule.second)) << pt(new_rule.first) << pt(new_rule.second));
                    current_rules.insert(new_rule);
                    actreenode->payload->insert(new_rule);
                } else {
                    ++i;
                }
            } else {
                ++i;
            }
        }
    }

    void trySimplify() noexcept {
        for (auto i = current_identities.begin(); i != current_identities.end();) {
            const auto &s1 = ss->get(i->first);
            const auto &s2 = ss->get(i->second);
            //auto s1reduced = reduceCopy(s1.begin(),s1.end());
            auto s1reduced = std::make_pair(s1, false);
            auto s2reduced = reduceCopy(s2.begin(), s2.end());
            if ((s1reduced.second && s1reduced.first.size() < max_allowed_string_size) || (s2reduced.second && s2reduced.first.size() < max_allowed_string_size)) {
                auto new_identity = *i;
                if (s1reduced.second) { debugline("trySimplify: rewriting " << toString(s1) << " to " << toString(s1reduced.first));
                    //assertss(sum(s1) == sum(s1reduced.first), pt(sum(s1)) << pt(sum(s1reduced.first)) );
                    new_identity.first = getOrCreateString(s1reduced.first);
                }
                if (s2reduced.second) { debugline("trySimplify: rewriting " << toString(s2) << " to " << toString(s2reduced.first));
                    //assertss(sum(s2) == sum(s2reduced.first), pt(sum(s2)) << pt(sum(s2reduced.first)) );
                    new_identity.second = getOrCreateString(s2reduced.first);
                }
                //assert(new_identity != *i); // we are supposed to have changed something remember...
                // this can apparently happen in cases where we would have gone in an infinite loop, that we forcefully interrupted...
                i = current_identities.erase(i);
                if (new_identity.first != new_identity.second) {
                    const auto new_eq = orderIdentity(new_identity);
                    assertIdentity(new_eq);
                    current_identities.insert(new_eq);
                }
            } else {
                ++i;
            }
        }
    }

    void tryOrient() noexcept {
        for (auto i = current_identities.begin(); i != current_identities.end();) {
            const auto &s1 = ss->get(i->first);
            const auto &s2 = ss->get(i->second);
            if (complexity_comparison(s1, s2)) {
                auto new_rule = std::make_pair(i->second, i->first);
                assertIdentity(new_rule);
                if (new_rule.first != new_rule.second) { debugline("tryOrient(1): adding rule " << toString(ss->get(new_rule.first)) << " --> " << toString(ss->get(new_rule.second)));
                    current_rules.insert(new_rule);
                    actree.getOrCreate(s2.begin(), s2.end()).insert(new_rule);
                    i = current_identities.erase(i);
                } else {
                    ++i;
                }
            } else if (complexity_comparison(s2, s1)) {
                auto new_rule = std::make_pair(i->first, i->second);
                assertIdentity(new_rule);
                if (new_rule.first != new_rule.second) { debugline("tryOrient(2): adding rule " << toString(ss->get(new_rule.first)) << " --> " << toString(ss->get(new_rule.second)));
                    current_rules.insert(new_rule);
                    actree.getOrCreate(s1.begin(), s1.end()).insert(new_rule);
                    i = current_identities.erase(i);
                } else {
                    ++i;
                }
            } else {
                ++i;
            }
        }
    }



    // bool orderingOnRules(const rule& st, const rule& lr) {
    //   return /*complexity_comparison(get(lr.first) , get(st.first)) ||*/ ( lr.first == st.first &&   complexity_comparison(get(lr.second) , get(st.second)) );
    // }

    // void tryCollapse(){ // "effondrement" in french.
    //   assert(false); // could not figure out the correct ordering on rules as described in the wiki-page.
    //   for (auto i = current_rules.begin(); i != current_rules.end();) {
    //     bool changed = false;
    //     actree.iterate_matches(get(i->first).begin(),
    // 			     get(i->first).end(),
    // 			       [&](rules & rs,
    // 				   const typename stringtype::iterator& posbegin,
    // 				   const typename stringtype::iterator& posend){


    // 				 for (const auto &r : rs) {
    // 				   if (orderingOnRules( *i, r )) { // this is not good. how do i know when s > l ...
    // 				     stringtype mmm(get(i->first).begin(), posbegin);
    // 				     mmm.insert(mmm.end(),get(r.second).begin(),get(r.second).end() );
    // 				     mmm.insert(mmm.end(),posend,get(i->first).end() );
    // 				     current_identities.insert(orderIdentity(std::make_pair( getOrCreateString(mmm), i->second)));
    // 				     //std::cerr << "tryCollapse: changed ts " << toString(get(i->first)) << " --> " << toString(get(i->second)) << " into " <<  toString(mmm) << " --> " << toString(get(i->second))  <<  " using lr " << toString(get(r.first)) << " --> " << toString(get(r.second)) << std::endl;
    // 				     rs.erase(r);
    // 				     changed = true;
    // 				     return false;


    // 				   }
    // 				 }
    // 				 return true;
    // 			       });
    //     if (changed) {
    // 	i = current_rules.erase(i);
    //     }else{
    // 	++i;
    //     }
    //   }
    // }

    // void tryObsolete(){ // could not understand tryCollapse, implemented this instead: remove rules that are obviously redundant
    //   for (auto i = current_rules.begin(); i != current_rules.end();) {
    //     // temporarily remove this rule from the actree, the effect should be that it will not be used for reductions...
    //     auto* ptr = actree.getNoCreate(get(i->first));
    //     assert(ptr);
    //     assert(ptr->find(*i) != ptr->end() );
    //     ptr->erase(*i);

    //     reduce(get(i->first),
    // 	     [&](bool changed,
    // 		 const typename std::list<symboltype>::iterator& begin,
    // 		 const typename std::list<symboltype>::iterator& end){
    // 	       if (changed && std::equal(begin,end, get(i->second).begin(), get(i->second).end(), eq )){
    // 		 // we can consider rule i to be obsolete!
    // 		 //std::cerr << "OBSOLETE RULE: " << toString(get(i->first)) << " --> " << toString(get(i->second)) << std::endl;
    // 		 i = current_rules.erase(i);
    // 	       }else{
    // 		 // not obsolete yet,
    // 		 ptr->insert(*i);
    // 		 ++i;
    // 	       }
    // 	     });
    //   }
    // }

    void tryCollapseAttempt2() noexcept {
        for (auto i = current_rules.begin(); i != current_rules.end();) {

            // temporarily remove this rule from the actree, the effect should be that it will not be used for reductions...
            auto *ptr = actree.getNoCreate(ss->get(i->first));
            assert(ptr);
            assert(ptr->find(*i) != ptr->end());
            ptr->erase(*i);

            reduce(ss->get(i->first),
                   [&](const bool changed,
                       const typename std::list<symboltype>::iterator &begin,
                       const typename std::list<symboltype>::iterator &end) {
                       if (changed &&
                           !std::equal(begin, end, ss->get(i->first).begin(), ss->get(i->first).end(), ss->eq)
                               ) {
                           //debugline("collapsing rule: " << toString(ss->get(i->first)) << " --> " << toString(ss->get(i->second)));debugline(
                           //        "collapsed rule became identity: " << toString(begin, end) << " == " << toString(ss->get(i->second)));
                           stringid news = getOrCreateString(begin, end);
                           if (ss->get(news).size() < max_allowed_string_size) {
                               equality new_identity(getOrCreateString(begin, end), i->second);
                               if (new_identity.first != new_identity.second) {
                                   new_identity = orderIdentity(new_identity);
                                   if (current_identities.find(new_identity) == current_identities.end()) { debugline(
                                               "collapsed rule became identity: " << toString(ss->get(new_identity.first)) << " == " << toString(ss->get(new_identity.second)));

                                       assertIdentity(new_identity);
                                       current_identities.insert(new_identity);
                                   }
                               }
                               i = current_rules.erase(i);
                           } else {
                               // not obsolete yet,
                               ptr->insert(*i);
                               ++i;
                           }
                       } else {
                           // not obsolete yet,
                           ptr->insert(*i);
                           ++i;
                       }
                   });


        }
    }

    void tryDeduce() noexcept {
        for (auto i = current_rules.begin(); i != current_rules.end(); ++i) {
            auto j = i;
            ++j;
            for (; j != current_rules.end(); ++j) {
                rule large = *i;
                rule small = *j;
                if (ss->get(large.first).size() < ss->get(small.first).size()) { // this is not about complexity, it is about length
                    std::swap(large, small);
                }

                int s1size = ss->get(large.first).size();
                int s2size = ss->get(small.first).size();

                if (s2size == 0 || s1size == 0) {
                    continue;
                }
                for (int offset = 1 - s2size; offset < s1size; ++offset) {
                    const auto &s1 = ss->get(large.first); // these need to be within the loop because the content of strings gets modified during the iterations. Though ... with a deque i dont expect much issues.
                    const auto &s2 = ss->get(small.first);
                    int l = s2.size();
                    if (offset < 0) {
                        l += offset;
                    }
                    if (offset + s2.size() > s1.size()) {
                        l -= (offset + (int) s2.size() - (int) s1.size());
                    }

                    if (((std::size_t) ((int) s1size) + ((int) s2size) - l) > max_allowed_string_size) {
                        continue;
                    }
                    //prt2(large.first, strings.size());
                    //prt6(offset, l, s1.size(), s2.size(), toString(s1),toString(s2));
                    assert(l);
                    const auto large_overlapbegin = s1.begin() + std::max(0, offset);
                    const auto large_overlapend = large_overlapbegin + l;
                    const auto small_overlapbegin = s2.begin() + std::max(0, -offset);
                    const auto small_overlapend = small_overlapbegin + l;
                    if (std::equal(large_overlapbegin,
                                   large_overlapend,
                                   small_overlapbegin,
                                   small_overlapend,
                                   ss->eq)) {
                        equality new_identity; // == a critical pair
                        if (offset < 0) {
                            stringtype cp1(s2.begin(), small_overlapbegin);
                            cp1.insert(cp1.end(), ss->get(large.second).begin(), ss->get(large.second).end());
                            stringtype cp2(ss->get(small.second));
                            cp2.insert(cp2.end(), large_overlapend, s1.end());
                            new_identity = std::make_pair(reduceCopyRegister(cp1), reduceCopyRegister(cp2));
                        } else if (offset + s2.size() > s1.size()) {

                            stringtype cp1(s1.begin(), large_overlapbegin);
                            cp1.insert(cp1.end(), ss->get(small.second).begin(), ss->get(small.second).end());
                            stringtype cp2(ss->get(large.second));
                            cp2.insert(cp2.end(), small_overlapend, s2.end());

                            new_identity = std::make_pair(reduceCopyRegister(cp1), reduceCopyRegister(cp2));

                        } else {
                            stringtype cp1(s1.begin(), s1.begin() + std::max(0, offset));
                            cp1.insert(cp1.end(), ss->get(small.second).begin(), ss->get(small.second).end());
                            cp1.insert(cp1.end(), s1.begin() + std::max(0, offset) + l, s1.end());
                            new_identity.first = reduceCopyRegister(ss->get(large.second));
                            new_identity.second = reduceCopyRegister(cp1);
                        }
                        if (ss->get(new_identity.first).size() > max_allowed_string_size || ss->get(new_identity.second).size() > max_allowed_string_size) {
                            continue;
                        }

                        new_identity = orderIdentity(new_identity);debugline("large rule: " << toString(ss->get(large.first)) << " --> " << toString(ss->get(large.second)));debugline(
                                "small rule: " << toString(ss->get(small.first)) << " --> " << toString(ss->get(small.second)));
                        if (new_identity.first != new_identity.second && current_identities.find(new_identity) == current_identities.end()) { debugline(
                                    "critical pair: " << toString(ss->get(new_identity.first)) << " == " << toString(ss->get(new_identity.second)));
                            assertIdentity(new_identity);
                            current_identities.insert(new_identity);
                            return;
                        }

                    }
                }
            }
        }
    }

    void introduceInputs() noexcept {
        for (auto r : input_identities) {
            auto grr = orderIdentity(r);
            if (grr.first != grr.second) {
                current_identities.insert(grr);
            }
        }
    }

    bool cycleOnce() noexcept {
        if (got_confluent_rewrite_system || converged) {
            return true;
        }
        introduceInputs(); // it helps to re-introduce the inputs at each cycle. (Is there a bug that causes us to loose information?)
        tryDelete();
        tryCompose();
        trySimplify();
        tryOrient();

        //tryCollapse();
        //tryObsolete();
        tryCollapseAttempt2();
        tryDeduce();
        {
            Bits128 state_hash;
            // copy all state in one contiguous slab of memory. The strings themselves dont go in there so it cant be heavy.
            std::vector<rule> data(current_rules.begin(), current_rules.end());
            data.insert(data.end(), current_identities.begin(), current_identities.end());
            MurmurHash3_x64_128(data.data(), data.size() * sizeof(rule), 42, &state_hash);
            if (hashed_states.find(state_hash) != hashed_states.end()) {
                converged = true;
                return false; // finished simulation, we had this state before.
            }
            hashed_states.insert(state_hash);
        }

        return true;
    }

    bool run(const int maxcycles = 1000) noexcept {
        int n = 0;
        while (true) {
            if (++n > maxcycles) {
                return false;
            }
            if (!cycleOnce()) {
                got_confluent_rewrite_system = current_rules.empty();
                return true;
            }
        }
        return current_identities.empty(); // if we did not manage to melt away all identities then we wont have a confluent rewriting system.
    }


};


#endif

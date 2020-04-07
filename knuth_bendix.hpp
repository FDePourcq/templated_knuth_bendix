#ifndef KNUTH_BENDIX_HPP
#define KNUTH_BENDIX_HPP

#include <cassert>
#include <list>
#include <deque>
#include "aho_corasick.hpp" // fast multi-string pattern search
#include "murmur3.h"
#include <memory>

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

template<typename symbolinfo, typename indextype>
struct StringStorage {
    typedef typename symbolinfo::symboltype symboltype;
    typedef typename symbolinfo::stringtype stringtype;
    std::deque <stringtype> strings; // all strings get stored here.
    std::deque <indextype> ordered_stringindexes; // ordered by their associated string. this is the lookup-map for strings.

    std::function<bool(const symboltype &, const symboltype &)> comp = [](const symboltype &a, const symboltype &b) { return a < b; };
    std::function<bool(const symboltype &, const symboltype &)> eq = [](const symboltype &a, const symboltype &b) { return a == b; };

    template<typename iteratortype>
    indextype getOrCreateString(const iteratortype &begin, const iteratortype &end) {
        // check ordered_stringindexes if it exists already, if not store it in strings and put the index in ordered_stringindexes. return the index.
        indextype ret = *insertOrderedUnique(ordered_stringindexes,
                                             [&]() { // constructor
                                                 indextype ret = strings.size();
                                                 strings.emplace_back(begin, end);
                                                 return ret;
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

        //std::cerr << "string index " << ret << " is " << toString(begin,end) << std::endl;
        return ret;
    }

    template<typename stringtype2>
    indextype getOrCreateString(const stringtype2 &s) {
        return getOrCreateString(s.begin(), s.end());
    }
};

template<typename symbolinfo, typename stringid>
struct KnuthBendixCompletion {
    typedef typename symbolinfo::symboltype symboltype;
    typedef typename symbolinfo::stringtype stringtype;
    typedef StringStorage<symbolinfo, stringid> stringstoragetype;
    stringstoragetype *ss;

    typedef std::pair <stringid, stringid> equality; // == identity
    typedef std::pair <stringid, stringid> rule;
    typedef std::set <equality> identities;
    typedef std::set <rule> rules;

    identities input_identities; // the inputs are stored to allow reprocessing them.
    identities current_identities; // these will be converted into rules eventually
    rules current_rules; // these are going to be rewritten till they converge ... if they converge at all ...
    aho_corasick::basic_trie<stringtype, rules> actree; // given a inputstring this will efficiently give all the rules that got triggered (and where...).

    KnuthBendixCompletion(stringstoragetype *ss_) :
            ss(ss_) {
    }

    std::function<bool(const stringtype &, const stringtype &)> complexity_comparison = [&](const stringtype &a, const stringtype &b) {
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

    std::set <Bits128> hashed_states; // anti-loop detection

    template<typename iteratortype>
    stringid getOrCreateString(const iteratortype &begin, const iteratortype &end) {
        return ss->getOrCreateString(begin, end);
    }

    template<typename stringtype2>
    stringid getOrCreateString(const stringtype2 &s) {
        return ss->getOrCreateString(s.begin(), s.end());
    }

    equality orderIdentity(const equality &eq) {
        if (complexity_comparison(ss->strings[eq.first], ss->strings[eq.second])) {
            return equality(eq.second, eq.first);
        }
        return eq;
    }

    void addIdentity(const stringtype &a, const stringtype &b) {
        const auto identity = orderIdentity(std::make_pair(getOrCreateString(a), getOrCreateString(b)));
        input_identities.insert(identity);
    }

    void tryDelete() {
        for (auto i = current_identities.begin(); i != current_identities.end();) {
            const auto &s1 = ss->strings[i->first];
            const auto &s2 = ss->strings[i->second];
            if (std::equal(s1.begin(), s1.end(), s2.begin(), ss->eq)) {
                i = current_identities.erase(i);
            } else {
                ++i;
            }
        }
    }

    template<typename iteratortype, typename callbacktype>
    void reduce(const iteratortype &begin, const iteratortype &end, const callbacktype &fct) {
        std::list <symboltype> ret(begin, end);
        bool changed = false;
        bool changed_local = false;
        //std::cerr << "[";
        // i must ... introduce loopdetection ...
        do {

            changed_local = false;
            actree.iterate_matches(ret.begin(),
                                   ret.end(),
                                   [&](const rules &r,
                                       const typename std::list<symboltype>::iterator &posbegin,
                                       const typename std::list<symboltype>::iterator &posend) {
                                       if (r.empty()) {
                                           return true;
                                       }
                                       const auto &replacement = ss->strings[r.begin()->second];
                                       //std::cerr << " from string " << std::string(ret.begin(), ret.end()) << " , particularly " << std::string(posbegin,posend) << "  can be replaced by " << std::string(replacement.begin(), replacement.end())  << std::endl;

                                       //std::cerr << " match is " << toString(posbegin,posend) <<std::endl;
                                       //std::cerr << "applying rule " << toString(strings[r.begin()->first]) << " --> " << toString(strings[r.begin()->second]) << " : " << toString(ret);
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

    template<typename stringtype, typename callbacktype>
    void reduce(const stringtype &s, const callbacktype &fct) {
        reduce(s.begin(), s.end(), fct);
    }


    // what about using std::optional as returntype?
    template<typename iteratortype>
    std::pair<std::list<symboltype>, bool> reduceCopy(const iteratortype &begin, const iteratortype &end) {
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
                                       const auto &replacement = ss->strings[r.begin()->second];
                                       //std::cerr << " match is " << toString(posbegin,posend) <<std::endl;
                                       //std::cerr << "applying rule " << toString(strings[r.begin()->first]) << " --> " << toString(strings[r.begin()->second]) << " : " << toString(ret);
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

    std::pair<std::list<symboltype>, bool> reduceCopy(const stringtype &s) {
        return reduceCopy(s.begin(), s.end());
    }


    template<typename iteratortype>
    stringid reduceCopyRegister(const iteratortype &begin, const iteratortype &end) {
        stringid ret;
        reduce(begin,
               end,
               [&](const bool,//changed,
                   const typename std::list<symboltype>::iterator &begin,
                   const typename std::list<symboltype>::iterator &end) {
                   ret = getOrCreateString(begin, end);
               });
        return ret;
    };

    template<typename stringtype>
    stringid reduceCopyRegister(const stringtype &s) {
        return reduceCopyRegister(s.begin(), s.end());
    }

    void tryCompose() {
        // this only modifies rules, but the actree needs to be updated in tandem.
        for (auto i = current_rules.begin(); i != current_rules.end();) {
            const auto &s1 = ss->strings[i->first];
            const auto &s2 = ss->strings[i->second];
            auto mmm = reduceCopy(s2.begin(), s2.end());
            if (mmm.second) {
                //std::cerr << "tryCompose: rewriting " << toString(s2) << " to " << toString(mmm.first) << std::endl;
                //assert(sum(s2) == sum(mmm.first));
                //assertss(sum(s2) == sum(mmm.first), pt(sum(s2)) << pt(sum(mmm.first)) );
                auto *actreenode = actree.getNodeOrCreate(s1);
                assert(actreenode->payload.get());
                actreenode->payload->erase(*i);
                rule new_rule = std::make_pair(i->first, getOrCreateString(mmm.first.begin(), mmm.first.end()));
                assert(new_rule.second != i->second); // we are supposed to have changed something remember...
                i = current_rules.erase(i);
                current_rules.insert(new_rule);
                actreenode->payload->insert(new_rule);
            } else {
                ++i;
            }
        }
    }

    void trySimplify() {
        for (auto i = current_identities.begin(); i != current_identities.end();) {
            const auto &s1 = ss->strings[i->first];
            const auto &s2 = ss->strings[i->second];
            //auto s1reduced = reduceCopy(s1.begin(),s1.end());
            auto s1reduced = std::make_pair(s1, false);
            auto s2reduced = reduceCopy(s2.begin(), s2.end());
            if (s1reduced.second || s2reduced.second) {
                auto new_identity = *i;
                if (s1reduced.second) {
                    //std::cerr << "trySimplify: rewriting " << toString(s1) << " to " << toString(s1reduced.first) << std::endl;
                    //assertss(sum(s1) == sum(s1reduced.first), pt(sum(s1)) << pt(sum(s1reduced.first)) );
                    new_identity.first = getOrCreateString(s1reduced.first);
                }
                if (s2reduced.second) {
                    //std::cerr << "trySimplify: rewriting " << toString(s2) << " to " << toString(s2reduced.first) << std::endl;
                    //assertss(sum(s2) == sum(s2reduced.first), pt(sum(s2)) << pt(sum(s2reduced.first)) );
                    new_identity.second = getOrCreateString(s2reduced.first);
                }
                assert(new_identity != *i); // we are supposed to have changed something remember...
                i = current_identities.erase(i);
                if (new_identity.first != new_identity.second) {
                    current_identities.insert(orderIdentity(new_identity));
                }
            } else {
                ++i;
            }
        }
    }

    void tryOrient() {
        for (auto i = current_identities.begin(); i != current_identities.end();) {
            const auto &s1 = ss->strings[i->first];
            const auto &s2 = ss->strings[i->second];
            if (complexity_comparison(s1, s2)) {
                //std::cerr << "tryOrient(1): adding rule " << toString(s2) << " --> " << toString(s1) <<  pt(s2.size()) << " " << pt(s1.size())  << std::endl;
                auto new_rule = std::make_pair(i->second, i->first);
                current_rules.insert(new_rule);
                actree.getOrCreate(s2.begin(), s2.end()).insert(new_rule);
                i = current_identities.erase(i);
            } else if (complexity_comparison(s2, s1)) {
                //std::cerr << "tryOrient(2): adding rule " << toString(s1) << " --> " << toString(s2) << std::endl;
                auto new_rule = std::make_pair(i->first, i->second);
                current_rules.insert(new_rule);
                actree.getOrCreate(s1.begin(), s1.end()).insert(new_rule);
                i = current_identities.erase(i);
            } else {
                ++i;
            }
        }
    }

    // bool orderingOnRules(const rule& st, const rule& lr) {
    //   return /*complexity_comparison(strings[lr.first] , strings[st.first]) ||*/ ( lr.first == st.first &&   complexity_comparison(strings[lr.second] , strings[st.second]) );
    // }

    // void tryCollapse(){ // "effondrement" in french.
    //   assert(false); // could not figure out the correct ordering on rules as described in the wiki-page.
    //   for (auto i = current_rules.begin(); i != current_rules.end();) {
    //     bool changed = false;
    //     actree.iterate_matches(strings[i->first].begin(),
    // 			     strings[i->first].end(),
    // 			       [&](rules & rs,
    // 				   const typename stringtype::iterator& posbegin,
    // 				   const typename stringtype::iterator& posend){


    // 				 for (const auto &r : rs) {
    // 				   if (orderingOnRules( *i, r )) { // this is not good. how do i know when s > l ...
    // 				     stringtype mmm(strings[i->first].begin(), posbegin);
    // 				     mmm.insert(mmm.end(),strings[r.second].begin(),strings[r.second].end() );
    // 				     mmm.insert(mmm.end(),posend,strings[i->first].end() );
    // 				     current_identities.insert(orderIdentity(std::make_pair( getOrCreateString(mmm), i->second)));
    // 				     //std::cerr << "tryCollapse: changed ts " << toString(strings[i->first]) << " --> " << toString(strings[i->second]) << " into " <<  toString(mmm) << " --> " << toString(strings[i->second])  <<  " using lr " << toString(strings[r.first]) << " --> " << toString(strings[r.second]) << std::endl;
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
    //     auto* ptr = actree.getNoCreate(strings[i->first]);
    //     assert(ptr);
    //     assert(ptr->find(*i) != ptr->end() );
    //     ptr->erase(*i);

    //     reduce(strings[i->first],
    // 	     [&](bool changed,
    // 		 const typename std::list<symboltype>::iterator& begin,
    // 		 const typename std::list<symboltype>::iterator& end){
    // 	       if (changed && std::equal(begin,end, strings[i->second].begin(), strings[i->second].end(), eq )){
    // 		 // we can consider rule i to be obsolete!
    // 		 //std::cerr << "OBSOLETE RULE: " << toString(strings[i->first]) << " --> " << toString(strings[i->second]) << std::endl;
    // 		 i = current_rules.erase(i);
    // 	       }else{
    // 		 // not obsolete yet,
    // 		 ptr->insert(*i);
    // 		 ++i;
    // 	       }
    // 	     });
    //   }
    // }

    void tryCollapseAttempt2() {
        for (auto i = current_rules.begin(); i != current_rules.end();) {

            // temporarily remove this rule from the actree, the effect should be that it will not be used for reductions...
            auto *ptr = actree.getNoCreate(ss->strings[i->first]);
            assert(ptr);
            assert(ptr->find(*i) != ptr->end());
            ptr->erase(*i);

            reduce(ss->strings[i->first],
                   [&](const bool changed,
                       const typename std::list<symboltype>::iterator &begin,
                       const typename std::list<symboltype>::iterator &end) {
                       if (changed && std::equal(begin, end, ss->strings[i->second].begin(), ss->strings[i->second].end(), ss->eq)) {
                           //std::cerr << "collapsing rule: " << toString(strings[i->first]) << " --> " << toString(strings[i->second]) << std::endl;
                           //std::cerr << "collapsed rule became identity: " << toString(reducedstuff.first) << " == " << toString(strings[i->second]) << std::endl;
                           equality new_identity(getOrCreateString(begin, end), i->second);
                           if (new_identity.first != new_identity.second) {
                               new_identity = orderIdentity(new_identity);
                               if (current_identities.find(new_identity) == current_identities.end()) {
                                   //std::cerr << "collapsed rule became identity: " << toString(strings[new_identity.first]) << " == " << toString(strings[new_identity.second]) << std::endl;
                                   current_identities.insert(new_identity);
                               }
                           }
                           i = current_rules.erase(i);
                       } else {
                           // not obsolete yet,
                           ptr->insert(*i);
                           ++i;
                       }
                   });


        }
    }

    void tryDeduce() {
        for (auto i = current_rules.begin(); i != current_rules.end(); ++i) {
            auto j = i;
            ++j;
            for (; j != current_rules.end(); ++j) {
                rule large = *i;
                rule small = *j;
                if (ss->strings[large.first].size() < ss->strings[small.first].size()) { // this is not about complexity, it is about length
                    std::swap(large, small);
                }

                int s1size = ss->strings[large.first].size();
                int s2size = ss->strings[small.first].size();

                for (int offset = 1 - s2size; offset < s1size; ++offset) {
                    const auto &s1 = ss->strings[large.first]; // these need to be within the loop because the content of strings gets modified during the iterations. Though ... with a deque i dont expect much issues.
                    const auto &s2 = ss->strings[small.first];
                    int l = s2.size();
                    if (offset < 0) {
                        l += offset;
                    }
                    if (offset + s2.size() > s1.size()) {
                        l -= (offset + (int) s2.size() - (int) s1.size());
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
                            cp1.insert(cp1.end(), ss->strings[large.second].begin(), ss->strings[large.second].end());
                            stringtype cp2(ss->strings[small.second]);
                            cp2.insert(cp2.end(), large_overlapend, s1.end());
                            new_identity = std::make_pair(reduceCopyRegister(cp1), reduceCopyRegister(cp2));
                        } else if (offset + s2.size() > s1.size()) {

                            stringtype cp1(s1.begin(), large_overlapbegin);
                            cp1.insert(cp1.end(), ss->strings[small.second].begin(), ss->strings[small.second].end());
                            stringtype cp2(ss->strings[large.second]);
                            cp2.insert(cp2.end(), small_overlapend, s2.end());

                            new_identity = std::make_pair(reduceCopyRegister(cp1), reduceCopyRegister(cp2));

                        } else {
                            stringtype cp1(s1.begin(), s1.begin() + std::max(0, offset));
                            cp1.insert(cp1.end(), ss->strings[small.second].begin(), ss->strings[small.second].end());
                            cp1.insert(cp1.end(), s1.begin() + std::max(0, offset) + l, s1.end());
                            new_identity.first = reduceCopyRegister(ss->strings[large.second]);
                            new_identity.second = reduceCopyRegister(cp1);
                        }
                        new_identity = orderIdentity(new_identity);
                        //std::cerr << "large rule: " << toString(strings[large.first]) << " --> " << toString(strings[large.second]) << std::endl;
                        //std::cerr << "small rule: " << toString(strings[small.first]) << " --> " << toString(strings[small.second]) << std::endl;
                        if (new_identity.first != new_identity.second && current_identities.find(new_identity) == current_identities.end()) {
                            //std::cerr << "critical pair: " << toString(strings[new_identity.first]) << " == " << toString(strings[new_identity.second]) << std::endl;
                            current_identities.insert(new_identity);
                        }

                    }
                }
            }
        }
    }

    void introduceInputs() {
        current_identities.insert(input_identities.begin(), input_identities.end());
    }

    bool cycleOnce() {

        introduceInputs();

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
            std::vector <rule> data(current_rules.begin(), current_rules.end());
            data.insert(data.end(), current_identities.begin(), current_identities.end());
            MurmurHash3_x64_128(data.data(), data.size() * sizeof(rule), 42, &state_hash);
            if (hashed_states.find(state_hash) != hashed_states.end()) {
                return false; // finished simulation, we had this state before.
            }
            hashed_states.insert(state_hash);
        }

        return true;
    }

    bool run(const int maxcycles = 1000) {
        int n = 0;
        while (true) {
            if (++n > maxcycles) {
                return false;
            }
            if (!cycleOnce()) {
                return true;
            }
        }
        return current_identities.empty(); // if we did not manage to melt away all identities then we wont have a confluent rewriting system.
    }


};


#endif

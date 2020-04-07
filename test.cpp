
#include "knuth_bendix.hpp"
#include <iostream>

#define prt(x) std::cerr << #x " = '" << x << "'" << std::endl;
#define prt2(x, y) std::cerr << #x " = '" << x << "'\t" << #y " = '" << y << "'" << std::endl;
#define prt3(x, y, z) std::cerr << #x " = '" << x << "'\t" << #y " = '" << y << "'\t" << #z " = '" << z << "'" << std::endl;
#define prt4(x, y, z, u) std::cerr << #x " = '" << x << "'\t" << #y " = '" << y << "'\t" << #z " = '" << z << "'\t" << #u " = '" << u << "'" << std::endl;
#define prt5(x, y, z, u, v) std::cerr << #x " = '" << x \
    << "'\t" << #y " = '" << y \
    << "'\t" << #z " = '" << z \
    << "'\t" << #u " = '" << u \
    << "'\t" << #v " = '" << v \
    << "'" << std::endl;
#define prt6(x, y, z, u, v, w) std::cerr << #x " = '" << x \
    << "'\t" << #y " = '" << y \
    << "'\t" << #z " = '" << z \
    << "'\t" << #u " = '" << u \
    << "'\t" << #v " = '" << v \
    << "'\t" << #w " = '" << w \
    << "'" << std::endl;
#define prt7(x, y, z, u, v, w, t) std::cerr << #x " = '" << x \
    << "'\t" << #y " = '" << y \
    << "'\t" << #z " = '" << z \
    << "'\t" << #u " = '" << u \
    << "'\t" << #v " = '" << v \
    << "'\t" << #w " = '" << w \
    << "'\t" << #t " = '" << t \
    << "'" << std::endl;

#define pt(x)  "\"" << #x << "\" = " << x << ";\t"

#define assertss(expr, value)\
   if (!(expr)){\
       std::cerr << "BOEM!" << std::endl << value << std::endl; \
        __assert_fail (__STRING(expr), __FILE__, __LINE__, __ASSERT_FUNCTION); abort(); \
   }
#define assertssexec(expr, value, __e__)\
   if (!(expr)){\
       std::cerr << "BOEM!" << std::endl << value << std::endl; __e__; \
        __assert_fail (__STRING(expr), __FILE__, __LINE__, __ASSERT_FUNCTION); abort(); \
   }


template<typename iteratortype>
std::string toString(const iteratortype &begin, const iteratortype &end) {
    std::ostringstream oss;
    for (auto i = begin; i != end; ++i) {
        oss << *i << " ";
    }
    return oss.str();
}

template<typename containertype>
std::string toString(const containertype &s) {
    return toString(s.begin(), s.end());
};

void test1() {
    // an example from wikipedia, meant for groups but works out without that logic.
    struct symbolinfo {
        typedef char symboltype;
        typedef std::vector<symboltype> stringtype;
    };

    StringStorage<symbolinfo, std::size_t> ss;
    KnuthBendixCompletion<symbolinfo, std::size_t> kbc(&ss);

    kbc.addIdentity({'1', 'x'}, {'x'});
    kbc.addIdentity({'1', 'y'}, {'y'});
    kbc.addIdentity({'x', '1'}, {'x'});
    kbc.addIdentity({'y', '1'}, {'y'});
    kbc.addIdentity({'x', 'x', 'x'}, {'1'});
    kbc.addIdentity({'y', 'y', 'y'}, {'1'});
    kbc.addIdentity({'x', 'y', 'x', 'y', 'x', 'y'}, {'1'});

    //prt4(kbc.ss->strings.size(),kbc.ordered_stringindexes.size(), kbc.current_identities.size(), kbc.current_rules.size());
    kbc.run();
    //prt4(kbc.ss->strings.size(),kbc.ordered_stringindexes.size(), kbc.current_identities.size(), kbc.current_rules.size());


    std::cerr << std::endl;
    const auto &strings = kbc.ss->strings;
    for (auto &i : kbc.current_rules) {
        std::cerr << toString(strings[i.first]) << " --> " << toString(strings[i.second]) << std::endl;
    }


}

void test3() {
    // Attempt to build multiple rewrite systems each with a different complexity ordering.
    // The intent is to minimise the amount of different symbols used.
    // When 2 arbitrary strings can be reformulated such that one becomes a substring of the other, then we know that they are not equivalent.
    //    And if we assume that all the symbols map to positive values then we can also conclude that the larger string is "more".

    struct symbolinfo {
        typedef char symboltype;
        typedef std::basic_string<symboltype> stringtype;
    };
    StringStorage<symbolinfo, std::size_t> ss;

    for (auto desired_symbols : std::vector<symbolinfo::stringtype>{"1", "2", "3", "4", "5", "9", "29", ""}) {
        std::cerr << " ----------------------- " << std::endl
                  << "desired symbols are " << toString(desired_symbols) << std::endl;
        KnuthBendixCompletion<symbolinfo, std::size_t> kbc(&ss); // got to specify the complexity-comparator still.

        auto countstuff = [&desired_symbols, &kbc](
                const symbolinfo::stringtype &a,
                int &goodcount,
                int &badcount) {
            for (char c : a) {
                bool bad = true;
                for (symbolinfo::symboltype desired : desired_symbols) {
                    if (kbc.ss->eq(c, desired)) {
                        bad = false;
                        ++goodcount;
                        break;
                    }
                }
                if (bad) {
                    ++badcount;
                }
            }
        };
        kbc.complexity_comparison = [&](const symbolinfo::stringtype &a,
                                        const symbolinfo::stringtype &b) {
            int bada = 0;
            int gooda = 0;
            countstuff(a, gooda, bada);
            int badb = 0;
            int goodb = 0;
            countstuff(b, goodb, badb);
            //prt4(a,b,bada,badb);
            if (bada < badb) {
                return true;
            }
            if (bada > badb) {
                return false;
            }
            if (gooda > goodb) {
                return true;
            }
            if (gooda < goodb) {
                return false;
            }
            if (a.size() < b.size())
                return true;
            if (a.size() > b.size())
                return false;
            return std::lexicographical_compare(a.begin(), a.end(), b.begin(), b.end(), kbc.ss->comp);
        };



        //kbc.addIdentity("3","111");
        kbc.addIdentity("12", "3");
        kbc.addIdentity("12", "21");
        //kbc.addIdentity("41","14");
        kbc.addIdentity("5", "32");
        kbc.addIdentity("4", "22");
        // kbc.addIdentity("111","3");
        //kbc.addIdentity("32","41");
        kbc.addIdentity("2", "11");
        kbc.addIdentity("54", "9");
        kbc.addIdentity("8", "53");
        kbc.run(5);


        std::cerr << std::endl;
        const auto &strings = kbc.ss->strings;
        for (auto &i : kbc.current_rules) {
            std::cerr << toString(strings[i.first]) << " --> " << toString(strings[i.second]) << std::endl;
        }
        for (auto &i : kbc.current_identities) {
            std::cerr << toString(strings[i.first]) << " == " << toString(strings[i.second]) << std::endl;
        }

        for (auto &test : std::vector<std::string>{"123459", "493", "33331", "12229", "8888", "5999"}) {
            std::cerr << "eg " << test << " reduces to " << toString(kbc.reduceCopy(test).first) << std::endl;
        }
    }
}

int main() {
    test1();
    test3();
}

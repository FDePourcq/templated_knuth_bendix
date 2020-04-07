# Templated knuth bendix completion algorithm 

This is an implementation for ... [see wikipedia](https://en.wikipedia.org/wiki/Knuth%E2%80%93Bendix_completion_algorithm).

It works with simple plain sequences of symbols and equalities between them.
- It doesn't understand parse trees or any sort of funny syntax. 
- It doesn't understand mathematical groups. 
- It doesn't know about any operators like inverse.

But it understands that equalities of strings can be used to rewrite strings.
In combination with a user-provided complexity ordering (like [shortlex](https://en.wikipedia.org/wiki/Shortlex_order)) it can ( but it is not guaranteed ) build a rewrite-system ( a set of rewrite-rules...) that is [confluent](https://en.wikipedia.org/wiki/Confluence_(abstract_rewriting)). (ie when rewriting a string with those rules you will end up with the same end-result irrespective of how you applied those rules).

Having a confluent rewrite system is very useful because you can use it to know when 2 strings are equivalent.
In other words this solves the [word problem](https://en.wikipedia.org/wiki/Word_problem_(mathematics)).


Besides knowing wether 2 arbitrary sequences are equivalent I also want to be able to know (in an automated way) which one is "more" than the other.  (This comes with the assumption that all symbols are positive, unable to negate eachother.)

> If we could rewrite strings a and b in such a way that a is a substring of (or equal to) b then we know that b is "more" than (or equal to) a.
     
To get to a situation whereby one string becomes a substring of the other it helps a lot to minimise the amount of symbols used.
By tweaking the complexity ordering it is sometimes possible to rewrite arbitrary sequences in such a way that they use only 1 or 2 symbols, greatly improving the probability that one will be a substring of the other.

Example
-----------
Consider identities

    "12" == "3"
    "12" == "21"
    "5"  == "32"
    "4"  == "22"
    "2"  == "11"
    "54" == "9"
    "8"  == "53"`
  
Question: is "33331" more than "493" ?

By tweaking the complexity ordering we can favor the symbol "1". This is the resulting rewrite system:

    3  --> 1 1 1 
    5  --> 1 1 1 1 1 
    4  --> 1 1 1 1 
    2  --> 1 1 
    9  --> 1 1 1 1 1 1 1 1 1 
    8  --> 1 1 1 1 1 1 1 1 
  
Result:

    493   reduces to 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 
    33331 reduces to 1 1 1 1 1 1 1 1 1 1 1 1 1 

Turns out "493" is more than "33331".

Download, compile, test
-----------

```shell
$ git clone https://github.com/FDePourcq/templated_knuth_bendix.git
Cloning into 'templated_knuth_bendix'...
remote: Enumerating objects: 12, done.
remote: Counting objects: 100% (12/12), done.
remote: Compressing objects: 100% (11/11), done.
remote: Total 12 (delta 2), reused 8 (delta 1), pack-reused 0
Unpacking objects: 100% (12/12), done.
$ cd templated_knuth_bendix/
$ git submodule init
Submodule 'generic_aho_corasick' (https://github.com/FDePourcq/generic_aho_corasick.git) registered for path 'generic_aho_corasick'
Submodule 'murmur3' (https://github.com/PeterScott/murmur3.git) registered for path 'murmur3'
$ git submodule update
Cloning into '/tmp/wii/templated_knuth_bendix/generic_aho_corasick'...
Cloning into '/tmp/wii/templated_knuth_bendix/murmur3'...
Submodule path 'generic_aho_corasick': checked out '318c513dfa46b48f1d8481b852a93cf473906c67'
Submodule path 'murmur3': checked out 'dae94be0c0f54a399d23ea6cbe54bca5a4e93ce4'
$ cmake .
-- The C compiler identification is GNU 8.3.0
-- The CXX compiler identification is GNU 8.3.0
-- Check for working C compiler: /usr/bin/cc
-- Check for working C compiler: /usr/bin/cc -- works
-- Detecting C compiler ABI info
-- Detecting C compiler ABI info - done
-- Detecting C compile features
-- Detecting C compile features - done
-- Check for working CXX compiler: /usr/bin/c++
-- Check for working CXX compiler: /usr/bin/c++ -- works
-- Detecting CXX compiler ABI info
-- Detecting CXX compiler ABI info - done
-- Detecting CXX compile features
-- Detecting CXX compile features - done
-- Configuring done
-- Generating done
-- Build files have been written to: /tmp/wii/templated_knuth_bendix
$ make -j4
Scanning dependencies of target test_knuth_bendix
[ 66%] Building C object CMakeFiles/test_knuth_bendix.dir/murmur3/murmur3.c.o
[ 66%] Building CXX object CMakeFiles/test_knuth_bendix.dir/test.cpp.o
[100%] Linking CXX executable test_knuth_bendix
[100%] Built target test_knuth_bendix
$ ./test_knuth_bendix 

1 x  --> x 
1 y  --> y 
x 1  --> x 
y 1  --> y 
x x x  --> 1 
y y y  --> 1 
1 1  --> 1 
y x y x  --> x x y y 
y y x x  --> x y x y 
 ----------------------- 
desired symbols are 1 

3  --> 1 1 1 
5  --> 1 1 1 1 1 
4  --> 1 1 1 1 
2  --> 1 1 
9  --> 1 1 1 1 1 1 1 1 1 
8  --> 1 1 1 1 1 1 1 1 
eg 123459 reduces to 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 
eg 493 reduces to 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 
eg 33331 reduces to 1 1 1 1 1 1 1 1 1 1 1 1 1 
eg 12229 reduces to 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 
eg 8888 reduces to 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 
eg 5999 reduces to 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 
 ----------------------- 
desired symbols are 2 

3  --> 1 2 
2 1  --> 1 2 
5  --> 1 2 2 
4  --> 2 2 
1 1  --> 2 
9  --> 1 2 2 2 2 
8  --> 2 2 2 2 
eg 123459 reduces to 2 2 2 2 2 2 2 2 2 2 2 2 
eg 493 reduces to 2 2 2 2 2 2 2 2 
eg 33331 reduces to 1 2 2 2 2 2 2 
eg 12229 reduces to 2 2 2 2 2 2 2 2 
eg 8888 reduces to 2 2 2 2 2 2 2 2 2 2 2 2 2 2 2 2 
eg 5999 reduces to 2 2 2 2 2 2 2 2 2 2 2 2 2 2 2 2 
 ----------------------- 
desired symbols are 3 

1 2  --> 3 
2 1  --> 3 
3 2  --> 2 3 
5  --> 2 3 
2 2  --> 1 3 
4  --> 1 3 
1 1  --> 2 
9  --> 3 3 3 
8  --> 2 3 3 
3 1  --> 1 3 
eg 123459 reduces to 3 3 3 3 3 3 3 3 
eg 493 reduces to 1 3 3 3 3 3 
eg 33331 reduces to 1 3 3 3 3 
eg 12229 reduces to 1 3 3 3 3 3 
eg 8888 reduces to 2 3 3 3 3 3 3 3 3 3 3 
eg 5999 reduces to 2 3 3 3 3 3 3 3 3 3 3 
 ----------------------- 
desired symbols are 4 

1 2  --> 3 
2 1  --> 3 
3 2  --> 1 4 
5  --> 1 4 
2 2  --> 4 
1 1  --> 2 
9  --> 1 4 4 
8  --> 4 4 
4 1  --> 1 4 
2 3  --> 1 4 
3 1  --> 4 
1 3  --> 4 
3 3  --> 2 4 
4 2  --> 2 4 
4 3  --> 3 4 
eg 123459 reduces to 4 4 4 4 4 4 
eg 493 reduces to 4 4 4 4 
eg 33331 reduces to 1 4 4 4 
eg 12229 reduces to 4 4 4 4 
eg 8888 reduces to 4 4 4 4 4 4 4 4 
eg 5999 reduces to 4 4 4 4 4 4 4 4 
 ----------------------- 
desired symbols are 5 

1 2  --> 3 
2 1  --> 3 
3 2  --> 5 
2 2  --> 4 
1 1  --> 2 
9  --> 4 5 
5 4  --> 4 5 
5 3  --> 3 5 
8  --> 3 5 
5 1  --> 1 5 
4 1  --> 5 
2 3  --> 5 
3 1  --> 4 
1 4  --> 5 
1 3  --> 4 
3 3  --> 1 5 
5 2  --> 2 5 
3 4  --> 2 5 
4 4  --> 3 5 
4 2  --> 1 5 
2 4  --> 1 5 
4 3  --> 2 5 
eg 123459 reduces to 4 5 5 5 5 
eg 493 reduces to 1 5 5 5 
eg 33331 reduces to 3 5 5 
eg 12229 reduces to 1 5 5 5 
eg 8888 reduces to 2 5 5 5 5 5 5 
eg 5999 reduces to 2 5 5 5 5 5 5 
 ----------------------- 
desired symbols are 9 

1 2  --> 3 
2 1  --> 3 
3 2  --> 5 
2 2  --> 4 
1 1  --> 2 
5 4  --> 9 
5 3  --> 8 
5 1  --> 1 5 
4 1  --> 5 
5 5  --> 1 9 
2 3  --> 5 
3 1  --> 4 
1 4  --> 5 
1 3  --> 4 
3 3  --> 1 5 
9 1  --> 1 9 
9 2  --> 2 9 
9 3  --> 3 9 
5 2  --> 2 5 
3 4  --> 2 5 
8 2  --> 1 9 
4 4  --> 8 
4 2  --> 1 5 
2 4  --> 1 5 
8 3  --> 2 9 
4 3  --> 2 5 
8 4  --> 9 3 
8 1  --> 9 
3 5  --> 8 
4 5  --> 9 
5 8  --> 1 9 3 
8 5  --> 4 9 
2 9 1  --> 9 3 
9 5  --> 5 9 
9 4  --> 4 9 
3 8  --> 2 9 
4 8  --> 9 3 
5 8 3  --> 8 8 
8 8  --> 4 9 3 
1 8  --> 9 
2 8  --> 1 9 
9 8  --> 8 9 
5 9 3  --> 9 8 
2 5 9 3  --> 2 9 8 
1 5 9 3  --> 1 9 8 
4 9 8  --> 9 3 9 
4 9 8  --> 9 9 3 
5 9 8  --> 1 9 9 3 
1 5 9 8  --> 2 9 9 3 
9 9 3  --> 9 3 9 
2 5 9 8  --> 9 3 9 3 
2 5 9 8  --> 3 9 9 3 
5 9 9 3  --> 9 9 8 
eg 123459 reduces to 1 5 9 9 
eg 493 reduces to 2 5 9 
eg 33331 reduces to 4 9 
eg 12229 reduces to 2 5 9 
eg 8888 reduces to 5 9 9 9 
eg 5999 reduces to 5 9 9 9 
 ----------------------- 
desired symbols are 2 9 

3  --> 1 2 
2 1  --> 1 2 
5  --> 1 2 2 
4  --> 2 2 
1 1  --> 2 
8  --> 2 2 2 2 
1 2 2 2 2  --> 9 
9 1  --> 2 2 2 2 2 
1 9  --> 2 2 2 2 2 
9 2  --> 2 9 
9 9  --> 2 2 2 2 2 2 2 2 2 
1 2 9  --> 2 2 2 2 2 2 
1 2 2 9  --> 2 2 2 2 2 2 2 
1 2 2 2 9  == 2 2 2 2 2 2 2 2 
eg 123459 reduces to 2 2 2 2 2 2 2 2 2 2 2 2 
eg 493 reduces to 2 2 2 2 2 2 2 2 
eg 33331 reduces to 2 2 9 
eg 12229 reduces to 1 2 2 2 9 
eg 8888 reduces to 2 2 2 2 2 2 2 2 2 2 2 2 2 2 2 2 
eg 5999 reduces to 2 2 2 2 2 2 2 2 2 2 2 2 2 2 2 2 
 ----------------------- 
desired symbols are 

1 2  --> 3 
2 1  --> 3 
3 2  --> 5 
2 2  --> 4 
1 1  --> 2 
5 4  --> 9 
5 3  --> 8 
5 1  --> 1 5 
4 1  --> 5 
5 5  --> 1 9 
2 3  --> 5 
3 1  --> 4 
1 4  --> 5 
1 3  --> 4 
3 3  --> 1 5 
9 1  --> 1 9 
9 2  --> 2 9 
9 3  --> 4 8 
5 2  --> 2 5 
3 4  --> 2 5 
8 2  --> 1 9 
4 4  --> 8 
4 2  --> 1 5 
2 4  --> 1 5 
8 3  --> 3 8 
4 3  --> 2 5 
8 4  --> 3 9 
8 1  --> 9 
3 5  --> 8 
4 5  --> 9 
1 5 5  --> 3 8 
2 5 5  --> 4 8 
5 8  --> 4 9 
8 5  --> 4 9 
9 5  --> 5 9 
9 4  --> 4 9 
3 8  --> 2 9 
4 8  --> 3 9 
1 8  --> 9 
2 8  --> 1 9 
2 5 9  --> 8 8 
9 8  --> 8 9 
2 5 9 9  --> 9 8 8 
2 5 9 8  --> 1 5 9 9 
1 5 9 9  --> 8 8 8 
2 9 8 8  --> 1 9 8 9 
1 5 5 5  --> 2 5 9 
1 9 8 8  --> 9 8 9 
8 8 8 8  == 5 9 9 9 
eg 123459 reduces to 8 8 8 
eg 493 reduces to 8 8 
eg 33331 reduces to 4 9 
eg 12229 reduces to 8 8 
eg 8888 reduces to 8 8 8 8 
eg 5999 reduces to 5 9 9 9
$
```




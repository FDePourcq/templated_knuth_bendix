# templated knuth bendix completion algorithm 

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

> > If i could rewrite strings a and b in such a way that a is a substring of (or equal to) b then I know that b is "more" than (or equal to) a.
     
To get to a situation whereby one string becomes a substring of the other it helps a lot to minimise the amount of symbols used.
By tweaking the complexity ordering it is sometimes possible to rewrite arbitrary sequences in such a way that they use only 1 or 2 symbols, greatly improving the probability that one will be a substring of the other.

Example :
-----------
consider identities

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

Turns out "493" is more than "33331"

  

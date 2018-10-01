# MISP

## Introduction

MIcro liSP (or Mosty Impractical liSP) exercise implementation in C for
microcontrollers. Challange to achieve minimal support and bootstrap rest
of the language in MISP itself.

Goal is to obtain bare minimum primitives(axioms) required to boostrap allowing rest of the
language to be implemented in MISP itself.

Highly influenced by [Lisp in Less Than 200 Lines in C](https://carld.github.io/2017/06/20/lisp-in-less-than-200-lines-of-c.html)
and [Make a Lisp](https://github.com/kanaka/mal).

## Basics

Target list of primitives:

1. (def! atom (expression))

   Defines new atom in curent scope.
   Expression will be evaluated and definition time.

2. (fn* (parameters) (expression))

   Defines new anonymous lambda expression. Parameters list may be empty.
   Parameters will be bound to scope derived from definition time scope, to
   those parameters values from call time will be bound, evaluated using call
   time scope.
   Expression will be evaluated using call time scope.

3. (cond? (pred1 value1) (pred2 value2) ... (predN valueN))

   Branching expressions. Will evaluate predicated till last one which returns
   true, then return evaluated value corresponding to that predicate. Otherwise
   returns empty list.

4. ()

   Evaluates to false in conditional expression, everything else evaluates to true.

5. ; comment

   Anything from semicolon (;) till end of the line will be treated as comment.

6. (int N) (len X)

   Numeric system in MISP is based on integers only.


For examples see core.mp file, which contains bootstrapped language definition.

Available online on [Misp on Repl.it](https://repl.it/@karolciba/MostlyImpracticalLisp).

## TODO List
* exceptions

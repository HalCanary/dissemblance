#!/bin/sh

GOOD=1
test() {
    Q="$1"
    A="$2"
    X="$(echo "$Q" | bin/dissemblance | tail -n 1)"
    if ! [ "$A" = "$X" ] ; then
        echo "'$Q' => '$X', not '$A'"
        GOOD=''
    fi
}

test '(if (quote T) (quote A) (quote B))' 'A'
test '(if () (quote A) (quote B))' 'B'
test '(define x 3) (+ x x)' '6'
test '(+)' '0'
test '(+ 99)' '99'
test '(* 99)' '99'
test '(* 99 10)' '990'
test '(* 99.5 10)' '995'
test '(+ 99 1.0e3)' '1099'
test '(+ 3 9 1)' '13'
test '(- 1)' '-1'
test '(- 3 1)' '2'
test '((lambda (x) (+ x x)) 5)' '10'
test '(define double (lambda (x) (+ x x))) (double 5)' '10'
test '((lambda (x y) (+ x x y)) 5 6)' '16'
test '(= 4 4)' '1'
test '(= 0 4)' '()'
test '(!= (- 2 2) 4)' '1'
test '(< 1 2)' '1'
test '(> 1 2)' '()'
test '(> 1 2)' '()'
test '(< 1 2)' '1'

#RECURSION!
test '(define fib (lambda (x) (if (= 0 x) 1 (* x (fib (- x 1)))))) (fib 5)' '120'

test '(define foo (lambda () (define bar 42) bar)) (foo)' '42'
test '(quote foo)' 'foo'
test '(quote (foo bar baz))' '(foo bar baz)'
test '(list 1 2 3)' '(1 2 3)'
test '(list 1 2 (quote foo) 4)' '(1 2 foo 4)'

if [ "$GOOD" ]; then
    echo good
else
    echo BAD
fi

#!/bin/sh

GOOD=1
test() {
    Q="$1"
    A="$2"
    X="$(echo "$Q" | bin/dissemblance)"
    if ! [ "$A" = "$X" ] ; then
        echo "$Q => $X, not $A"
        GOOD=''
    fi
}

test '(if (quote T) (quote A) (quote B))' 'A'
test '(if () (quote A) (quote B))' 'B'
test '(begin (define x 3) (+ x x))' '6'
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
test '(begin (define double (lambda (x) (+ x x))) (double 5))' '10'
test '((lambda (x y) (+ x x y)) 5 6)' '16'
test '(== 4 4)' '1'
test '(== 0 4)' '()'
test '(!= (- 2 2) 4)' '1'
test '(< 1 2)' '1'
test '(> 1 2)' '()'
test '(> 1 2)' '()'
test '(< 1 2)' '1'

if [ "$GOOD" ]; then
    echo good
else
    echo BAD
fi

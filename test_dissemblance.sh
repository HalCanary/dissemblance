#!/bin/sh

GOOD=1
test() {
    Q="$1"
    A="$2"
    X="$(echo "$Q" | bin/dissemblance | tail -n 1)"
    if ! [ "$A" = "$X" ] ; then
        echo "\"$Q\" => \"$X\", not \"$A\""
        GOOD=''
    fi
}
test2() {
    Q="$(cat)"
    A="$1"
    X="$(echo "$Q" | bin/dissemblance | tail -n 1)"
    if ! [ "$A" = "$X" ] ; then
        echo "\"$Q\" => \"$X\", not \"$A\""
        GOOD=''
    fi
}

test '(if (quote T) (quote A) (quote B))' 'A'
test '(if () (quote A) (quote B))' 'B'
test "(if 'T 'A 'B)" "A"
test "(if () 'A 'B)" "B"
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
test '(define fact (lambda (x) (if (= 0 x) 1 (* x (fact (- x 1)))))) (fact 5)' '120'

test '(define foo (lambda () (define bar 42) bar)) (foo)' '42'
test '(quote foo)' 'foo'
test '(quote (foo bar baz))' '(foo bar baz)'
test "'foo" "foo"
test "'(foo bar baz)" "(foo bar baz)"
test '(list 1 2 3)' '(1 2 3)'
test "(list 1 2 'foo 4)" '(1 2 foo 4)'
test '(cons 1 2)' '(1 . 2)'
test '(cons 1 ())' '(1)'
test '(cons () ())' '(())'
test '(cons 1 (cons 2 (cons 3 ())))' '(1 2 3)'

test '(car (cons 1 2))' '1'
test '(car (cons 1 ()))' '1'
test '(car (cons () ()))' '()'
test '(car (cons 1 (cons 2 (cons 3 ()))))' '1'

test '(cdr (cons 1 2))' '2'
test '(cdr (cons 1 ()))' '()'
test '(cdr (cons () ()))' '()'
test '(cdr (cons 1 (cons 2 (cons 3 ()))))' '(2 3)'

test2 120 << EOF
(define fact
  (lambda (x)
   (if (= 0 x)
       1
       (* x (fact (- x 1))))))
(fact 5)
EOF

test2 13 << EOF
(define fib
  (lambda (i)
      (define fib-recursive (lambda (a b i)
        (if (= i 0)
            b
            (fib-recursive b (+ a b) (- i 1)))))
      (fib-recursive 0 1 i)))
(fib 6)
EOF

if [ "$GOOD" ]; then
    echo good
else
    echo BAD
fi

#!/bin/sh

GOOD=1
test() {
    Q="$(cat)"
    A="$1"
    X="$(echo "$Q" | bin/dissemblance | tail -n 1)"
    if ! [ "$A" = "$X" ] ; then
        echo "\"$Q\" => \"$X\", not \"$A\""
        GOOD=''
    fi
}

echo '(if (quote T) (quote A) (quote B))' | test 'A'
echo '(if () (quote A) (quote B))' | test 'B'
echo "(if 'T 'A 'B)" | test "A"
echo "(if () 'A 'B)" | test "B"
echo '(define x 3) (+ x x)' | test '6'
echo '(+)' | test '0'
echo '(+ 99)' | test '99'
echo '(* 99)' | test '99'
echo '(* 99 10)' | test '990'
echo '(* 99.5 10)' | test '995'
echo '(+ 99 1.0e3)' | test '1099'
echo '(+ 3 9 1)' | test '13'
echo '(- 1)' | test '-1'
echo '(- 3 1)' | test '2'
echo '((lambda (x) (+ x x)) 5)' | test '10'
echo '(define double (lambda (x) (+ x x))) (double 5)' | test '10'
echo '((lambda (x y) (+ x x y)) 5 6)' | test '16'
echo '(= 4 4)' | test '1'
echo '(= 0 4)' | test '()'
echo '(!= (- 2 2) 4)' | test '1'
echo '(< 1 2)' | test '1'
echo '(> 1 2)' | test '()'
echo '(> 1 2)' | test '()'
echo '(< 1 2)' | test '1'

#RECURSION!
test '120' <<EOF
(define fact
        (lambda (x)
                (if (= 0 x)
                    1
                    (* x (fact (- x 1))))))
(fact 5)
EOF

echo '(define foo (lambda () (define bar 42) bar)) (foo)' | test '42'
echo '(quote foo)' | test 'foo'
echo '(quote (foo bar baz))' | test '(foo bar baz)'
echo "'foo" | test "foo"
echo "'(foo bar baz)" | test "(foo bar baz)"
echo '(list 1 2 3)' | test '(1 2 3)'
echo "(list 1 2 'foo 4)" | test '(1 2 foo 4)'
echo '(cons 1 2)' | test '(1 . 2)'
echo '(cons 1 ())' | test '(1)'
echo '(cons () ())' | test '(())'
echo '(cons 1 (cons 2 (cons 3 ())))' | test '(1 2 3)'

echo '(car (cons 1 2))' | test '1'
echo '(car (cons 1 ()))' | test '1'
echo '(car (cons () ()))' | test '()'
echo '(car (cons 1 (cons 2 (cons 3 ()))))' | test '1'

echo '(cdr (cons 1 2))' | test '2'
echo '(cdr (cons 1 ()))' | test '()'
echo '(cdr (cons () ()))' | test '()'
echo '(cdr (cons 1 (cons 2 (cons 3 ()))))' | test '(2 3)'

test 120 << EOF
(define fact
  (lambda (x)
   (if (= 0 x)
       1
       (* x (fact (- x 1))))))
(fact 5)
EOF

test 13 << EOF
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

#!/bin/bash
assert() {
	expected="$1"
	input="$2"

	./n9cc "$input" > tmp.s
	cc -o tmp tmp.s
	./tmp
	actual="$?"

	if [ "$actual" = "$expected" ]; then
		echo "$input => $actual"
	else
		echo "$input => $expected expected, but got $actual"
		exit 1
	fi
}

cc -o n9cc main.c

assert 0 "0;"
assert 255 "255;"
assert 42 "42;"

assert 41 " 12 + 34 - 5 ; "
assert 47 "5+6*7;"
assert 15 "5*(9-6);"
assert 4 "(3+5)/2;"
assert 10 "-10+20;"
assert 10 "- -10;"
assert 10 "- - +10;"

assert 0 "0==1;"
assert 1 "42==42;"
assert 1 "0!=1;"
assert 0 "42!=42;"

assert 1 "0<1;"
assert 0 "1<1;"
assert 0 "2<1;"
assert 1 "0<=1;"
assert 1 "1<=1;"
assert 0 "2<=1;"

assert 1 "1>0;"
assert 0 "1>1;"
assert 0 "1>2;"
assert 1 "1>=0;"
assert 1 "1>=1;"
assert 0 "1>=2;"

assert 100 "a=100;"
assert 100 "a=b=100;"
assert 10 "a=100;b=10;"
assert 10 "a=100;a-90;"
assert 10 "a=b=100;b-90;"
assert 10 "a=90;b=100;b-a;"

echo OK

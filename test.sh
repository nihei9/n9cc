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
assert 10 "foo=90;bar=100;bar-foo;"

assert 1 "0; return 1;"
assert 1 "return 1; return 0;"
assert 1 "returnx=1;" # 'returnx' is an identifier, not 'return' keyword.
assert 1 "return1;"

assert 0 "a=100; if(a > 0) return 0; return 1;"
assert 1 "a=100; if(a > 100) return 0; return 1;"
assert 0 "a=100; b=200; if(a > 0) if(b > 100) return 0; return 1;"
assert 1 "a=100; b=200; if(a > 0) if(b > 200) return 0; return 1;"
assert 1 "a=100; if(a > 0) 0; return 1;"

assert 0 "a=100; if(a > 0) return 0; else return 1;"
assert 1 "a=-1; if(a > 0) return 0; else return 1;"
assert 8 "a=0; b=10; if(a > 0) return 9; else if (b <= 10) return 8; else return 7;"
assert 7 "a=0; b=10; if(a > 0) return 9; else if (b < 10) return 8; else return 7;"

echo OK

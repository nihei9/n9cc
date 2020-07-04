#!/bin/bash
assert() {
	expected="$1"
	input="$2"

	./n9cc "$input" > tmp.s
	cc -o tmp tmp.s helper.c
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
assert 11 "a=10; a = a + 1;"

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

assert 10 "a=0; while(a < 10) a = a + 1; return a;"
assert 10 "a=0; while(1) if(a < 10) a = a + 1; else return a; return 0;"
assert 0 "while(0) return 1; return 0;"
assert 10 "a=0; while(1) if (a >= 10) break; else a = a + 1; return a;"
assert 10 "a=0; while(1) {if (a >= 10) {break;} else {a = a + 1;}} return a;"
assert 10 "{a=0; while(1) {if (a >= 10) {break;} else {a = a + 1;}} return a;}"

assert 10 "for(a = 0; a < 10; a = a + 1) 0; return a;"
assert 10 "a=0; for(; a < 10; a = a + 1) 0; return a;"
assert 10 "for(a = 0; ; a = a + 1) if (a >= 10) return a; return 0;"
assert 10 "for(a = 0; a < 10; ) a = a + 1; return a;"
assert 10 "a = 0; for(;;) if (a < 10) a = a + 1; else return a;"
assert 10 "a = 0; for(;;) if (a >= 10) break; else a = a + 1; return a;"
assert 10 "a = 0; for(;;) {if (a >= 10) {break;} else {a = a + 1;}} return a;"
assert 10 "{a = 0; for(;;) {if (a >= 10) {break;} else {a = a + 1;}} return a;}"

assert 0 "{} return 0;"
assert 0 "{return 0;}"
assert 1 "{0; return 1;}"
assert 10 "a=0; while(1) {if (a >= 10) return a; a = a + 1;}"
assert 10 "for (i=0; i < 10; i = i + 1) {} return i;"

assert 42 "ret42();"
assert 42 "id(42);"
assert 42 "add2(41, 1);"
assert 42 "add3(39, 2, 1);"
assert 42 "add4(36, 3, 2, 1);"
assert 42 "add5(32, 4, 3, 2, 1);"
assert 42 "add6(27, 5, 4, 3, 2, 1);"

echo OK

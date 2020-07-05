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

assert 0 "int main(){return 0;}"
assert 255 "int main(){255;}"
assert 42 "int main(){42;}"

assert 41 "int main(){ 12 + 34 - 5 ; }"
assert 47 "int main(){5+6*7;}"
assert 15 "int main(){5*(9-6);}"
assert 4 "int main(){(3+5)/2;}"
assert 10 "int main(){-10+20;}"
assert 10 "int main(){- -10;}"
assert 10 "int main(){- - +10;}"

assert 0 "int main(){0==1;}"
assert 1 "int main(){42==42;}"
assert 1 "int main(){0!=1;}"
assert 0 "int main(){42!=42;}"

assert 1 "int main(){0<1;}"
assert 0 "int main(){1<1;}"
assert 0 "int main(){2<1;}"
assert 1 "int main(){0<=1;}"
assert 1 "int main(){1<=1;}"
assert 0 "int main(){2<=1;}"

assert 1 "int main(){1>0;}"
assert 0 "int main(){1>1;}"
assert 0 "int main(){1>2;}"
assert 1 "int main(){1>=0;}"
assert 1 "int main(){1>=1;}"
assert 0 "int main(){1>=2;}"

assert 100 "int main(){int a; a=100;}"
assert 100 "int main(){int a; int b; a=b=100;}"
assert 10 "int main(){int a; int b; a=100;b=10;}"
assert 10 "int main(){int a; a=100; return a-90;}"
assert 10 "int main(){int a; int b; a=b=100;b-90;}"
assert 10 "int main(){int a; int b; a=90;b=100;b-a;}"
assert 10 "int main(){int foo; int bar; foo=90;bar=100;bar-foo;}"
assert 11 "int main(){int a; a=10; a = a + 1;}"

assert 1 "int main(){0; return 1;}"
assert 1 "int main(){return 1; return 0;}"

assert 0 "int main(){int a; a=100; if(a > 0) return 0; return 1;}"
assert 1 "int main(){int a; a=100; if(a > 100) return 0; return 1;}"
assert 0 "int main(){int a; int b; a=100; b=200; if(a > 0) if(b > 100) return 0; return 1;}"
assert 1 "int main(){int a; int b; a=100; b=200; if(a > 0) if(b > 200) return 0; return 1;}"
assert 1 "int main(){int a; a=100; if(a > 0) 0; return 1;}"

assert 0 "int main(){int a; a=100; if(a > 0) return 0; else return 1;}"
assert 1 "int main(){int a; a=-1; if(a > 0) return 0; else return 1;}"
assert 8 "int main(){int a; int b; a=0; b=10; if(a > 0) return 9; else if (b <= 10) return 8; else return 7;}"
assert 7 "int main(){int a; int b; a=0; b=10; if(a > 0) return 9; else if (b < 10) return 8; else return 7;}"

assert 10 "int main(){int a; a=0; while(a < 10) a = a + 1; return a;}"
assert 10 "int main(){int a; a=0; while(1) if(a < 10) a = a + 1; else return a; return 0;}"
assert 0 "int main(){while(0) return 1; return 0;}"
assert 10 "int main(){int a; a=0; while(1) if (a >= 10) break; else a = a + 1; return a;}"
assert 10 "int main(){int a; a=0; while(1) {if (a >= 10) {break;} else {a = a + 1;}} return a;}"
assert 10 "int main(){{int a; a=0; while(1) {if (a >= 10) {break;} else {a = a + 1;}} return a;}}"

assert 10 "int main(){int a; for(a = 0; a < 10; a = a + 1) 0; return a;}"
assert 10 "int main(){int a; a=0; for(; a < 10; a = a + 1) 0; return a;}"
assert 10 "int main(){int a; for(a = 0; ; a = a + 1) if (a >= 10) return a; return 0;}"
assert 10 "int main(){int a; for(a = 0; a < 10; ) a = a + 1; return a;}"
assert 10 "int main(){int a; a = 0; for(;;) if (a < 10) a = a + 1; else return a;}"
assert 10 "int main(){int a; a = 0; for(;;) if (a >= 10) break; else a = a + 1; return a;}"
assert 10 "int main(){int a; a = 0; for(;;) {if (a >= 10) {break;} else {a = a + 1;}} return a;}"
assert 10 "int main(){{int a; a = 0; for(;;) {if (a >= 10) {break;} else {a = a + 1;}} return a;}}"

assert 0 "int main(){{} return 0;}"
assert 0 "int main(){return 0;}"
assert 1 "int main(){0; return 1;}"
assert 10 "int main(){int a; a=0; while(1) {if (a >= 10) return a; a = a + 1;}}"
assert 10 "int main(){int i; for (i=0; i < 10; i = i + 1) {} return i;}"

assert 42 "int main(){ret42();}"
assert 42 "int main(){id(42);}"
assert 42 "int main(){add2(41, 1);}"
assert 42 "int main(){add3(39, 2, 1);}"
assert 42 "int main(){add4(36, 3, 2, 1);}"
assert 42 "int main(){add5(32, 4, 3, 2, 1);}"
assert 42 "int main(){add6(27, 5, 4, 3, 2, 1);}"

assert 42 "int r42(){return 42;} int main(){return r42();}"
assert 42 "int r20(){return 20;} int r22(){return 22;} int main(){return r20() + r22();}"
assert 42 "int sub(int a, int b){return a - b;} int main(){return sub(100, 58);}"
assert 12 "int fib(int n){if (n == 0) {return 0;} else if (n == 1) {return 1;} return fib(n - 1) + fib(n -2);} int main(){int n; int i; n = 0; for (i = 0; i <= 5; i = i + 1) {n = n + fib(i);} return n;}"

assert 42 "int main(){int a; int b; a=42; b=&a; return *b;}"
assert 42 "int main(){int a; int b; int c; a=42; b=&a; c=&b; return **c;}"

echo OK

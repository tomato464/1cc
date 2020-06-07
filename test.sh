#!/bin/bash

cat <<EOF | gcc -xc -c -o tmp2.o -
int add3(int a, int b, int c){return a + b + c;}
int add2(int a, int b){return a+b;}
int add6(int a, int b, int c, int d, int e, int f){ return a + b + c + d + e + f; };
int ret(int a){ return a;}
int ret3(){ return 3;}
int ret5(){ return 5;}
EOF

assert(){
	expected="$1"
	input="$2"

	./1cc "$input" > tmp.s || exit
	gcc -static -o tmp tmp.s tmp2.o
	./tmp
	actual="$?"

	if [ "$actual" = "$expected" ]; then
		echo "$input => $actual"

	else
		echo "$input => $expected expected, but got $actual"
		exit 1
	fi
}

assert 7 "{ return add2(3, 4); }"
assert 6 "return add3(1, 2, 3);"
assert 21 "{ return add6(1, 2, 3, 4, 5, 6); }"
assert 3 "{ return ret(3); }"
assert 10 "x = 10;for(i = 0; i < 10; i = i + 1)x = x + 1; return i;"
assert 42 "{return 42; }"
assert 0 "{ return 0; }"
assert 3 "{ return ret3(); }"
assert 5 "{ return ret5(); }"
assert 21 "return 5+20-4;"
assert 21 "return 5 + 20 - 4;"
assert 2  "return 3*4-10;"
assert 2 "return 3 * 4 - 10;"
assert 18  "return 3*4 + 2*3;"
assert 72  "return 3* (4+2)*4;"
assert 10  "return -10 + 20;"
assert 0 "return 0 == 1;"
assert 1 "return 42 == 42;"
assert 1 "return 3 != 5;"
assert 0 "return 6 != 6;"
assert 1 "return 5 > 3;"
assert 0 "return 5 > 6;"
assert 1 "return 4 >= 2;"
assert 1 "return 4 >= 4;"
assert 0 "return 5 >= 7;"
assert 1 "return 4 < 5;"
assert 0 "return 5 < 5;"
assert 0 "return 6 < 2;"
assert 1 "return 43 <= 53;"
assert 1 "return 42 <= 42;"
assert 0 "return 53 <= 21;"
assert 50 "b = 50; return b;" 
assert 14 "a = 4; b = 10; return a + b;"
assert 14 "foo = 4; two = 10; return foo + two;"
assert 43 "x = 11; y = 32; return x + y;"
assert 42 "x = 32; return 42; return 30;"
assert 43 "if(2 == 2)return 43;"
assert 42 "x = 42;if(x == 42)return 42;"
assert 42 "x = 42; if(x == 10)x = 10;return x;"
assert 20 "x = 42; if(x == 50)x = 10;if(x == 42)x = 20; return x;"
assert 43 "x = 10; if(x == 5)x = 20; else x = 43; return x;"
assert 20 "x = 5; if(x == 5) x = 20; else x = 43;return x;"
assert 30 "x = 5;y = 2; if(x == 5)if(y == 2) x = 30; return x;"
assert 10 "i = 0; while(i < 10)i = i + 1; return i;"
assert 20 "x = 10;for(i = 0; i < 10; i = i + 1)x = x + 1; return x;"
assert 10 "i = 0;for(;i < 10;i = i + 1)i = i + 1;return i;"
assert 10 "i = 0;for(;i < 10;)i = i + 1;return i;"
assert 30 "x = 43; if(x == 43){ x = x + 1; if(x != 43){x = 30;return x;}else{x = 50; return x;}}"
assert 40 "x = 10;if(x == 10){x = 20;y = 20; x = x + y;}return x;"
assert 10 "x = 10;if(x != 10){x = 20;y = 20; x = x + y;}return x;"
echo OK

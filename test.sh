#!/bin/bash
assert(){
	expected="$1"
	input="$2"

	./1cc "$input" > tmp.s
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

assert 30 "x = 43; if(x == 43){ x = x + 1; if(x != 43){x = 30;return x;}else{x = 50; return x;}}"
assert 40 "x = 10;if(x == 10){x = 20;y = 20; x = x + y;}return x;"
assert 10 "x = 10;if(x != 10){x = 20;y = 20; x = x + y;}return x;"
assert 0 "0;"
assert 42 "42;"
assert 21 "5+20-4;"
assert 21 "5 + 20 - 4;"
assert 2  "3*4-10;"
assert 2 "3 * 4 - 10;"
assert 18  "3*4 + 2*3;"
assert 72  "3* (4+2)*4;"
assert 10  "-10 + 20;"
assert 0 "0 == 1;"
assert 1 "42 == 42;"
assert 1 "3 != 5;"
assert 0 "6 != 6;"
assert 1 "5 > 3;"
assert 0 "5 > 6;"
assert 1 "4 >= 2;"
assert 1 "4 >= 4;"
assert 0 "5 >= 7;"
assert 1 "4 < 5;"
assert 0 "5 < 5;"
assert 0 "6 < 2;"
assert 1 "43 <= 53;"
assert 1 "42 <= 42;"
assert 0 "53 <= 21;"
assert 50 "b = 50; b;" 
assert 14 "a = 4; b = 10; a + b;"
assert 14 "foo = 4; two = 10; foo + two;"
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
assert 10 "x = 10;for(i = 0; i < 10; i = i + 1)x = x + 1; return i;"
assert 20 "x = 10;for(i = 0; i < 10; i = i + 1)x = x + 1; return x;"
assert 10 "i = 0;for(;i < 10;i = i + 1)i = i + 1;return i;"
assert 10 "i = 0;for(;i < 10;)i = i + 1;return i;"
echo OK

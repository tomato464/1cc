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

echo OK

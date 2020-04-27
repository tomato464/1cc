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


assert 0 0
assert 42 42
assert 21 "5+20-4"
assert 21 "5 + 20 - 4"
assert 2  "3*4-10"
assert 2 "3 * 4 - 10"
assert 18  "3*4 + 2*3"
assert 72  "3* (4+2)*4"
assert 10  "-10 + 20"

echo OK

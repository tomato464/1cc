#!/bin/bash

cat <<EOF | gcc -xc -c -o tmp2.o -
int add3(int a, int b, int c){return a + b + c;}
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

assert 3 "main(){ x = 3; y = &x; return *y;  }"
assert 5  "main(){ x = 5; y = 6; z = &y + 8; return *z;  }"
assert 43 "main(){ return add2(40, 3); } add2(a, b){ return a + b; }"
assert 55 "main(){ return fi(9); } fi(x){ if(x <= 1){ return 1; }else{ return fi(x - 1) + fi(x - 2); }  } "
assert 20 "main(){ return sub2(30, 10);  } sub2(a, b){ return a - b; }"
assert 14 "main(){a = 4; b = 10; return a + b;}"
assert 43 "main(){ return ret43(); } ret43(){return 43;}"
assert 0 "main(){ return 0; }"
assert 6 "main(){return add3(1, 2, 3);}"
assert 21 "main(){ return add6(1, 2, 3, 4, 5, 6); }"
assert 3 "main(){ return ret(3); }"
assert 10 "main(){x = 10;for(i = 0; i < 10; i = i + 1)x = x + 1; return i;}"
assert 42 "main(){return 42; }"
assert 3 "main(){ return ret3(); }"
assert 5 "main(){ return ret5(); }"
assert 21 "main(){return 5+20-4;}"
assert 21 "main(){return 5 + 20 - 4;}"
assert 2  "main(){return 3*4-10;}"
assert 2 "main(){return 3 * 4 - 10;}"
assert 18  "main(){return 3*4 + 2*3;}"
assert 72  "main(){return 3* (4+2)*4;}"
assert 10  "main(){return -10 + 20;}"
assert 0 "main(){return 0 == 1;}"
assert 1 "main(){return 42 == 42;}"
assert 1 "main(){return 3 != 5;}"
assert 0 "main(){return 6 != 6;}"
assert 1 "main(){return 5 > 3;}"
assert 0 "main(){return 5 > 6;}"
assert 1 "main(){return 4 >= 2;}"
assert 1 "main(){return 4 >= 4;}"
assert 0 "main(){return 5 >= 7;}"
assert 1 "main(){return 4 < 5;}"
assert 0 "main(){return 5 < 5;}"
assert 0 "main(){return 6 < 2;}"
assert 1 "main(){return 43 <= 53;}"
assert 1 "main(){return 42 <= 42;}"
assert 0 "main(){return 53 <= 21;}"
assert 50 "main(){b = 50; return b;}" 
assert 14 "main(){foo = 4; two = 10; return foo + two;}"
assert 43 "main(){x = 11; y = 32; return x + y;}"
assert 42 "main(){x = 32; return 42; return 30;}"
assert 43 "main(){if(2 == 2)return 43;}"
assert 42 "main(){x = 42;if(x == 42)return 42;}"
assert 42 "main(){x = 42; if(x == 10)x = 10;return x;}"
assert 20 "main(){x = 42; if(x == 50)x = 10;if(x == 42)x = 20; return x;}"
assert 43 "main(){x = 10; if(x == 5)x = 20; else x = 43; return x;}"
assert 20 "main(){x = 5; if(x == 5) x = 20; else x = 43;return x;}"
assert 30 "main(){x = 5;y = 2; if(x == 5)if(y == 2) x = 30; return x;}"
assert 10 "main(){i = 0; while(i < 10)i = i + 1; return i;}"
assert 20 "main(){x = 10;for(i = 0; i < 10; i = i + 1)x = x + 1; return x;}"
assert 10 "main(){i = 0;for(;i < 10;i = i + 1)i = i + 1;return i;}"
assert 10 "main(){i = 0;for(;i < 10;)i = i + 1;return i;}"
assert 30 "main(){x = 43; if(x == 43){ x = x + 1; if(x != 43){x = 30;return x;}else{x = 50; return x;}}}"
assert 40 "main(){x = 10;if(x == 10){x = 20;y = 20; x = x + y;}return x;}"
assert 10 "main(){x = 10;if(x != 10){x = 20;y = 20; x = x + y;}return x;}"
echo OK

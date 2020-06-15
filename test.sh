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

assert 0 "int main(){ int x[2][3]; int *y = x; *y = 0; return **x;}"
assert 1 "int main(){ int x[2][3]; int *y = x; *(y + 1) = 1; return *(*x + 1);}"
assert 2 "int main(){ int x[2][3]; int *y = x; *(y + 2) = 2; return *(*x + 2);}"
assert 3 "int main(){ int x[2][3]; int *y = x; *(y + 3) = 3; return **(x + 1);}"
assert 4 "int main(){ int x[2][3]; int *y = x; *(y + 4) = 4; return *(*(x + 1) + 1);}"
assert 5 "int main(){ int x[2][3]; int *y = x; *(y + 5) = 5; return *(*(x + 1) + 2);}"
assert 5 "int main(){ int x[3]; *x = 3; *(x + 1) = 4; *(x + 2) = 5; return *(x + 2);}"
assert 4 "int main(){ int x[3]; *x = 3; *(x + 1) = 4; *(x + 2) = 5; return *(x + 1);}"
assert 3 "int main(){ int x[3]; *x = 3; *(x + 1) = 4; *(x + 2) = 5; return *x;}"
assert 3 "int main(){ int x[2]; int *y = &x; *y = 3; return *x;}"
assert 7 "int main(){ int x = 3; int y = 5; *(&x + 1) = 7; return y;}"
assert 10 "int main(){int x; x = 10;int i; for(i = 0; i < 10; i = i + 1)x = x + 1; return i;}"
assert 21 "int main(){ return add6(1, 2, 3, 4, 5, 6); }"
assert 3 "int main(){ return ret3(); }"
assert 5 "int main(){ return ret5(); }"
assert 50 "int main(){int b; b = 50; return b;}" 
assert 18  "int main(){return 3*4 + 2*3;}"
assert 8 " int main(){ int x, y; x = 3; y = 5; return x + y;}"
assert 0 "int main(){ return 0; }"
assert 3 "int main(){ int x = 3; return *&x; }"
assert 3 "int main(){ int x = 3; int *y = &x; int **z = &y; return **z; }"
assert 5 "int main(){ int x = 3; int y = 5; return *(&x + 1);}"
assert 8 "int main(){ int x = 3, y = 5; return x + y;}"
assert 5 "int main(){ int x; int *y; y = &x; *y = 5; return x;}"
assert 43 "int main(){ return add2(40, 3); } int add2(int a, int b){ return a + b; }"
assert 55 "int main(){ return fi(9); } int fi(int x){ if(x <= 1){ return 1; }else{ return fi(x - 1) + fi(x - 2); }  } "
assert 20 "int main(){ return sub2(30, 10);  } int sub2(int a, int b){ return a - b; }"
assert 14 "int main(){int a; a = 4; int b; b = 10; return a + b;}"
assert 43 "int main(){ return ret43(); } int ret43(){return 43;}"
assert 6 "int main(){return add3(1, 2, 3);}"
assert 42 "int main(){return 42; }"
assert 21 "int main(){return 5+20-4;}"
assert 21 "int main(){return 5 + 20 - 4;}"
assert 2  "int main(){return 3*4-10;}"
assert 2 "int main(){return 3 * 4 - 10;}"
assert 72  "int main(){return 3* (4+2)*4;}"
assert 10  "int main(){return -10 + 20;}"
assert 0 "int main(){return 0 == 1;}"
assert 1 "int main(){return 42 == 42;}"
assert 1 "int main(){return 3 != 5;}"
assert 0 "int main(){return 6 != 6;}"
assert 1 "int main(){return 5 > 3;}"
assert 0 "int main(){return 5 > 6;}"
assert 1 "int main(){return 4 >= 2;}"
assert 1 "int main(){return 4 >= 4;}"
assert 0 "int main(){return 5 >= 7;}"
assert 1 "int main(){return 4 < 5;}"
assert 0 "int main(){return 5 < 5;}"
assert 0 "int main(){return 6 < 2;}"
assert 1 "int main(){return 43 <= 53;}"
assert 1 "int main(){return 42 <= 42;}"
assert 0 "int main(){return 53 <= 21;}"
assert 14 "int main(){int foo; foo = 4; int two; two = 10; return foo + two;}"
assert 43 "int main(){int x; x = 11;int y; y = 32; return x + y;}"
assert 42 "int main(){int x; x = 32; return 42; return 30;}"
assert 43 "int main(){if(2 == 2)return 43;}"
assert 42 "int main(){int x; x = 42;if(x == 42)return 42;}"
assert 42 "int main(){int x; x = 42; if(x == 10)x = 10;return x;}"
assert 20 "int main(){int x; x = 42; if(x == 50)x = 10;if(x == 42)x = 20; return x;}"
assert 43 "int main(){int x; x = 10; if(x == 5)x = 20; else x = 43; return x;}"
assert 20 "int main(){int x; x = 5; if(x == 5) x = 20; else x = 43;return x;}"
assert 30 "int main(){int x; x = 5;int y; y = 2; if(x == 5)if(y == 2) x = 30; return x;}"
assert 10 "int main(){int i; i = 0; while(i < 10)i = i + 1; return i;}"
assert 20 "int main(){int x; int i; x = 10;for(i = 0; i < 10; i = i + 1)x = x + 1; return x;}"
assert 10 "int main(){int i; i = 0;for(;i < 10;i = i + 1)i = i + 1;return i;}"
assert 10 "int main(){int i; i = 0;for(;i < 10;)i = i + 1;return i;}"
assert 30 "int main(){int x; x = 43; if(x == 43){ x = x + 1; if(x != 43){x = 30;return x;}else{x = 50; return x;}}}"
assert 40 "int main(){int x; int y; x = 10;if(x == 10){x = 20;y = 20; x = x + y;}return x;}"
assert 10 "int main(){int x; int y; x = 10;if(x != 10){x = 20;y = 20; x = x + y;}return x;}"
echo OK

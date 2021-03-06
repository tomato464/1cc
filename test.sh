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

	echo "$input" | ./1cc - > tmp.s || exit
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

assert 2 'int main(){int x = 2; {int x = 3; } return x;}'
assert 2 'int main(){int x = 2; {int x = 3;} { int y = 4; return x;}}'
assert 3 'int main(){ int x = 2; { x = 3;} return x;}'

assert 2 'int main(){ /* return 1; */ return 2;}'
assert 2 'int main(){// return 1;
			return 2;}'

assert 0 'int main(){ return ({ 0;}); }'
assert 2 'int main(){ return ({ 0; 1; 2; }); }'
assert 1 'int main(){ return ({ 0; return 1; 2; }); return 3;}'
assert 6 'int main(){ return ({ 1;}) + ({ 2; }) + ({ 3;}); }'
assert 3 'int main(){ return ({ int x= 3; x;}); }'

assert 0 'int main(){ return "\x00"[0]; }'
assert 119 'int main(){ return "\x77"[0]; }'
assert 165 'int main(){ return "\xA5"[0]; }'
assert 255 'int main(){ return "\x00ff"[0]; } '

assert 0 'int main(){ return "\0"[0]; }'
assert 16 'int main(){ return "\20"[0];}'
assert 65 'int main(){ return "\101"[0];}'
assert 104 'int main(){ return "\1500"[0];}'

assert 7 'int main(){ return "\a"[0]; }'
assert 8 'int main(){ return "\b"[0]; }'
assert 9 'int main(){ return "\t"[0]; }'
assert 10 'int main(){ return "\n"[0]; }'
assert 11 'int main(){ return "\v"[0]; }'
assert 12 'int main(){ return "\f"[0]; }'
assert 13 'int main(){ return "\r"[0]; }'
assert 27 'int main(){ return "\e"[0]; }'
assert 106 'int main(){ return "\j"[0]; }'
assert 107 'int main(){ return "\k"[0]; }'
assert 108 'int main(){ return "\l"[0]; }'
assert 7 'int main(){ return "\ax\ny"[0]; }'
assert 120 'int main(){ return "\ax\ny"[1]; }'
assert 10 'int main(){ return "\ax\ny"[2]; }'
assert 121 'int main(){ return "\ax\ny"[3]; }'

assert 97  'int main(){ return "abc"[0];}'
assert 98 'int main(){ return "abc"[1];}'
assert 99 'int main(){ return "abc"[2];}'
assert 0 'int main(){ return "abc"[3];}'
assert 4 'int main(){ return sizeof( "abc" ); }'

assert 1 "int main(){ char x = 1; return sizeof(x);}"
assert 1 "int main(){ char x = 1; char y = 2; return x;}"
assert 2 "int main(){ char x = 1; char y = 2; return y;}"
assert 1 "int main(){char x; return sizeof(x);}"
assert 10 "int main(){ char x[10]; return sizeof(x);}"
assert 1 "int main(){return sub_char(7, 3, 3);} int sub_char(char a, char b, char c){return a - b - c;}"

assert 0 "int x; int main(){return x;}"
assert 3 "int x; int main(){ x = 3; return x;}"
assert 7 "int x; int y; int main(){ x = 3; y =4; return x + y;}"
assert 7 "int x, y; int main(){ x = 3; y = 4; return x + y;}"
assert 0 "int x[4]; int main(){ x[0]=0; x[1] = 1; x[2] = 2; x[3] = 3;return x[0]; } " 
assert 1 "int x[4]; int main(){ x[0]=0; x[1] = 1; x[2] = 2; x[3] = 3;return x[1]; } " 
assert 2 "int x[4]; int main(){ x[0]=0; x[1] = 1; x[2] = 2; x[3] = 3;return x[2]; } " 
assert 3 "int x[4]; int main(){ x[0]=0; x[1] = 1; x[2] = 2; x[3] = 3;return x[3]; } " 
assert 8 "int x; int main(){return sizeof(x);}"
assert 32 "int x[4]; int main(){ return sizeof(x); }"

assert 8 "int main(){int x; return sizeof(x);}"
assert 8 "int main(){int x; return sizeof x ;}"
assert 8 "int main(){int *x; return sizeof(x);}"
assert 32 "int main(){int x[4]; return sizeof(x);}"
assert 96 "int main(){int x[3][4]; return sizeof(x);}"
assert 32 "int main(){int x[3][4]; return sizeof(*x);}"
assert 8 "int main(){int x[3][4]; return sizeof(**x);}"
assert 9 "int main(){int x[3][4]; return sizeof(**x) + 1;}"
assert 8 "int main(){int x[3][4]; return sizeof(**x + 1);}"
assert 8 "int main(){int x = 1; return sizeof(x = 2); }"
assert 1 "int main(){int x = 1; sizeof(x = 2); return x;}"

assert 0 "int main(){ int x[2][3]; int *y = x; y[0] = 0; return x[0][0];}"
assert 1 "int main(){ int x[2][3]; int *y = x; y[1] = 1; return x[0][1];}"
assert 2 "int main(){ int x[2][3]; int *y = x; y[2] = 2; return x[0][2];}"
assert 3 "int main(){ int x[2][3]; int *y = x; y[3] = 3; return x[1][0];}"
assert 4 "int main(){ int x[2][3]; int *y = x; y[4] = 4; return x[1][1];}"
assert 5 "int main(){ int x[2][3]; int *y = x; y[5] = 5; return x[1][2];}"

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
assert 3 "int x; int main(){x = 3; return x;}"
echo OK

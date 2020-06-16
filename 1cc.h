#define _POSIX_C_SOURCE 200809L
#include <assert.h>
#include <ctype.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct Type Type;

//token.c
typedef enum
{
	TK_RESERVED,	//記号
	TK_IDENT,	//変数
	TK_NUM,		//整数トークン
	TK_EOF,		//入力の終わりを表すトークン
}Tokenkind;

typedef struct Token Token;

//トークン型
struct Token
{
	Tokenkind kind;	//トークンの型
	Token *next;	//次の入力トークン
	int val;	//kindがTK_NUMの場合、その値
	char *loc;	//トークンのロケーション
	int len;	//トークン文字列の長さ
};


//エラーを報告するための関数
//printfと同じ引数を取る
void error(char *fmt, ...);
void error_tok(Token *tok, char *fmt, ...);

bool equal(Token *tok, char *op);
Token *skip(Token *tok, char *op);
bool consume(Token **rest, Token *tok, char *op);

//入力文字列pをトークナイズしてそれを返す
Token *tokenize();

//parser.c
typedef enum{
	ND_ADD,		// +
	ND_SUB,		// -
	ND_MUL,		// *
	ND_DIV,		// /
	ND_ADDR,	// unary &
	ND_DEREF,	// unary *
	ND_EQ,		// ==
	ND_NE,		// !=
	ND_LT,		// <
	ND_LE,		// <=
	ND_ASSIGN,	// =
	ND_RETURN,	// return
	ND_FUNCALL,	//function call
	ND_IF,		//if
	ND_ELSE,	// else
	ND_WHILE,	//while
	ND_FOR,		//for
	ND_BLOCK,	// {...}
	ND_VAR,	// Local変数
	ND_EXPR_STMT,	// Expression statement
	ND_NUM,		// integer
} Nodekind;

typedef struct Node Node;

//Variable
typedef struct Var Var;

struct Var
{
	Var *next;	//次の変数がNULL
	char *name;	//変数の名前
	Type *ty;	//type
	int offset;	//RBPからのオフセット
	bool is_local;	// local or global
};


struct Node
{
	Nodekind kind;	//ノードの種類
	Node *next;	//次のノード
	Type *ty;	// Type, ()
	Token *tok;	//Representative Token

	Node *lhs;	//左辺
	Node *rhs;	//右辺
	//for, if, whileの時、使う
	Node *cond;
	Node *then;
	Node *els;
	Node *inc;
	Node *init;

	// {...}の中身を格納
	Node *body;

	// function callの時に使う、関数名
	char *funcname;
	Node *args;

	
	long val;	//ND_NUMの時のみ使う

	Var *var;	// lond == ND_LVARの時に使う
};

typedef struct Function Function;

struct Function
{
	Function *next;
	char *name;
	Var *params;
	
	Node *node;
	Var *locals;
	int stack_size;
};

typedef struct{
	Var *globals;
	Function *fns;
} Program;

Program *parse(Token *tok);

//type.c

typedef enum {
	TY_INT,
	TY_CHAR,
	TY_PTR,
	TY_FUNC,
	TY_ARRAY,
} Typekind;

struct Type {
	Typekind kind;

	int size;	//sizeof() value
	
	//Pointer
	Type *base;

	// Declaration
	Token *name;

	// Array
	int array_len;

	//Function type
	Type *return_ty;
	Type *params;
	Type *next;
};

extern Type *ty_int;
extern Type *ty_char;

bool is_integer(Type *ty);

Type *copy_type(Type *ty);

Type *pointer_to(Type *base);

Type *func_type(Type *return_ty);

Type *array_of(Type *base, int size);

void add_type(Node *node);

//codegne.c
//コードを生成
void codegen(Program *prog);

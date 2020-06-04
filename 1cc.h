#define _POSIX_C_SOURCE 200809L
#include <assert.h>
#include <ctype.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

//token.c
typedef enum
{
	TK_RESERVED,	//記号
	TK_IDENT,	//変数
	TK_RETURN,	//returnトークン
	TK_IF,		// ifトークン
	TK_ELSE,	// elseトークン
	TK_WHILE,	// whie
	TK_FOR,		// for
	TK_BLOCK,	// {}
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

//入力文字列pをトークナイズしてそれを返す
Token *tokenize();

//parser.c
typedef enum{
	ND_ADD,		// +
	ND_SUB,		// -
	ND_MUL,		// *
	ND_DIV,		// /
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
	ND_LVAR,	// Local変数
	ND_EXPR_STMT,	// Expression statement
	ND_NUM,		// integer
} Nodekind;

typedef struct Node Node;

struct Node
{
	Nodekind kind;	//ノードの種類
	Node *lhs;	//左辺
	Node *rhs;	//右辺
	Node *next;	//次のノード
	Token *tok;	//Representative Token

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

	int val;	//ND_NUMの時のみ使う
	int offset;	//kindがND_LVARの時にのみ使う
};

typedef struct LVar LVar;

struct LVar
{
	LVar *next;	//次の変数がNULL
	char *name;	//変数の名前
	int len;	//名前の長さ
	int offset;	//RBPからのオフセット
};

//ローカル変数
LVar *locals;

//最初は+,-の処理を構文木を使って処理できるようにする
//void program(Token *tok);
Node *parse(Token *tok);

//codegne.c
//コードを生成
void codegen(Node *node);

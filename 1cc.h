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
	char *str;	//トークン文字列
	int len;	//トークン文字列の長さ
};

//入力された文字列
char *user_input;

//現在着目しているトークン
Token *token;

//エラーを報告するための関数
//printfと同じ引数を取る
void error(char *fmt, ...);

void error_at(char *loc, char *fmt, ...);


void expect(char *op);
//次のトークンが期待している記号の時には、トークンを一つ進めて
//真を返す。それ以外の時には偽を返す
bool consume(char *op);

bool consume_return(void);

bool consume_if(void);

bool consume_else(void);

bool consume_while(void);

bool consume_for(void);

Token *consume_ident(void);

//次のトークンが数値の場合、トークンを一つ進めてその数値を返す
//それ以外の場合にはエラーを報告する
int expect_number();

bool at_eof();

//新しいトークンを生成してcurに繋げる
Token *new_token(Tokenkind kind, Token *cur, char *str, int len);

bool is_alpha(char c);

bool is_alnum(char c);

bool is_it(char *op);

bool startswith(char *p, char *q);

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
	ND_IF,		//if
	ND_ELSE,	// else
	ND_WHILE,	//while
	ND_FOR,		//for
	ND_BLOCK,	// {...}
	ND_LVAR,	// Local変数
	ND_NUM,		// integer
} Nodekind;

typedef struct Node Node;

struct Node
{
	Nodekind kind;	//ノードの種類
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

	int val;	//ND_NUMの時のみ使う
	int offset;	//kindがND_LVARの時にのみ使う
};
Node *code[100];

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

LVar *find_lvar(Token *tok);

Node *new_node(Nodekind kind);

Node *new_binary(Nodekind kind, Node *lhs, Node *rhs);

Node *new_num(int val);

//最初は+,-の処理を構文木を使って処理できるようにする
void program();
Node *stmt();
Node *expr();
Node *assign();
Node *equality();
Node *relational();
Node *add();
Node *mul();
Node *unary();
Node *primary();

//codegne.c
//コードを生成
void gen_lval(Node *node);
void gen(Node *node);

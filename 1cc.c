#include <ctype.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>


//トークンのタイプ
typedef enum
{
	TK_RESERVED,	//記号
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
void error(char *fmt, ...)
{
	va_list ap;
	va_start(ap, fmt);
	vfprintf(stderr, fmt, ap);
	fprintf(stderr, "\n");
	exit(1);
}

void error_at(char *loc, char *fmt, ...)
{
	va_list ap;
	va_start(ap, fmt);

	int pos = loc - user_input;
	fprintf(stderr, "%s\n", user_input);
	fprintf(stderr, "%*s", pos, " ");
	fprintf(stderr, "^");

	vfprintf(stderr, fmt, ap);
	fprintf(stderr, "\n");
	exit(1);
}

//次のトークンが期待している記号の時には、トークンを一つ進めて
//真を返す。それ以外の時には偽を返す
bool consume(char *op)
{
	if(token->kind != TK_RESERVED || strlen(op) != token->len ||
		memcmp(token->str, op, token->len)){
		return false;
	}
	token = token->next;
	return true;
}

//次のトークンが期待している記号の時には、トークンを一つ読み進める。
//それ以外の場合にはエラーを報告する。
void expect(char *op)
{
	if(token->kind != TK_RESERVED || strlen(op) != token->len ||
		memcmp(token->str, op, token->len)){
		error_at(token->str, "'%c'ではありません", op);
	}
	token = token->next;
}

//次のトークンが数値の場合、トークンを一つ進めてその数値を返す
//それ以外の場合にはエラーを報告する
int expect_number(){
	if(token->kind != TK_NUM){
		error_at(token->str, "数ではありません");
	}
	int val = token->val;
	token = token->next;
	return val;
}

bool at_eof()
{
	return token->kind == TK_EOF;
}

//新しいトークンを生成してcurに繋げる
Token *new_token(Tokenkind kind, Token *cur, char *str, int len)
{
	Token *tok = calloc(1, sizeof(Token));
	tok->kind = kind;
	tok->str = str;
	tok->len = len;
	cur->next = tok;
	return tok;
}

bool startswith(char *p, char *q)
{
	return memcmp(p, q, strlen(q)) == 0;//p==qでtrueを返す
}

//入力文字列pをトークナイズしてそれを返す
Token *tokenize()
{
	char *p = user_input;
	Token head;
	head.next = NULL;
	Token *cur = &head;

	while(*p){
		//空白文字をスキップ
		if(isspace(*p)){
			p++;
			continue;
		}
		//複数文字
		if(startswith(p, "==") || startswith(p, "!=") ||
			startswith(p, ">=") || startswith(p, "<=")){
			cur = new_token(TK_RESERVED, cur, p, 2);
			p += 2;
		}

		if(strchr("+-*/()<>", *p)){
			cur = new_token(TK_RESERVED, cur, p, 1);
			p++;
			continue;
		}

		if(isdigit(*p)){
			cur = new_token(TK_NUM, cur, p, 0);//とりあえず0を入れておく
			char *q = p;
			cur->val = strtol(p, &p, 10);//pの値を更新
			cur->len = p - q;
			continue;
		}

		error_at(p, "数でない");
	}
	new_token(TK_EOF, cur, p, 0);
	return head.next;
}

typedef enum{
	ND_ADD,//+
	ND_SUB,//-
	ND_MUL,//*
	ND_DIV,// /
	ND_NUM,//integer
} Nodekind;

typedef struct Node Node;

struct Node
{
	Nodekind kind;	//ノードの種類
	Node *lhs;	//左辺
	Node *rhs;	//右辺
	int val;	//ND_NUMの時のみ使う
};

Node *new_node(Nodekind kind)
{
	Node *node = calloc(1, sizeof(Node));
	node->kind = kind;
	return node;
}

Node *new_binary(Nodekind kind, Node *lhs, Node *rhs)
{
	Node *node = new_node(kind);
	node->lhs = lhs;
	node->rhs = rhs;
	return node;
}

Node *new_num(int val)
{
	Node *node = new_node(ND_NUM);
	node->val = val;
	return node;
}

//最初は+,-の処理を構文木を使って処理できるようにする
Node *expr();
Node *mul();
Node *unary();
Node *primary();

// expr = mul ("+" mul | "-"mul)*
Node *expr()
{
	Node *node = mul();
	
	for(;;){
		if(consume("+")){
			node = new_binary(ND_ADD, node, mul());
		}
		else if(consume("-")){
			node = new_binary(ND_SUB, node, mul());
		}
		else{
			return node;
		}
	}
}

//mul = unary ("*"unary | "/"unary)*
Node *mul()
{
	Node *node = unary();

	for(;;){
		if(consume("*")){
			node = new_binary(ND_MUL, node, unary());
		}
		else if(consume("/")){
			node = new_binary(ND_DIV, node, unary());
		}
		else{
			return node;
		}
	}
}

//unary = ('+' | '-')?primary
Node *unary()
{
	if(consume("+")){
		return primary();
	}
	else if(consume("-")){
		return new_binary(ND_SUB, new_num(0), primary()); 
	}
	else{
		return primary();
	}
	
}

//primary = num | "("expr")"
Node *primary()
{
	if(consume("(")){
		Node *node = expr();
		consume(")");
		return node;
	}

	return new_num(expect_number());
}

//コードを生成
void gen(Node *node)
{
	if(node->kind == ND_NUM){
		printf("	push	%d\n", node->val);
		return;
	}

	gen(node->lhs);
	gen(node->rhs);

	printf("	pop	rdi\n");
	printf("	pop	rax\n");

	switch(node->kind){
		case ND_ADD:
			printf("	add	rax,rdi\n");
			break;
		case ND_SUB:
			printf("	sub	rax,rdi\n");
			break;
		case ND_MUL:
			printf("	imul	rax,rdi\n");
			break;
		case ND_DIV:
			printf("	cqo\n");
			printf("	idiv	rdi\n");
			break;
	}
	printf("	push	rax\n");
	return;
}

int main(int argc, char **argv)
{
	if(argc != 2){
		error("%s: 引数が正しくありません。\n", argv[0]);
		return 1;
	}
	
	//トークナイズする
	user_input = argv[1];
	token = tokenize();
	Node *node = expr();

	printf(".intel_syntax noprefix\n");
	printf(".global main\n");
	printf("main:\n");

	gen(node);

	printf("	pop	rax\n");
	printf("	ret\n");
	return 0;
}

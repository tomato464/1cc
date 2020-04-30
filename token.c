#include "1cc.h"
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

bool consume_return(void)
{
	if(token->kind != TK_RETURN){
		return false;
	}
	token = token->next;
	return true;
}

//次のトークンがローカル変数ならば、トークンを一つ進めてその変数を返す。（ローマ字）
//それ以外の時はNoneを返すl 
Token *consume_ident(void)
{
	if(token->kind != TK_IDENT){
		return NULL;
	}
	Token *local_value = token;
	token = token->next;
	return local_value;
}

//次のトークンが期待している記号の時には、トークンを一つ読み進める。
//それ以外の場合にはエラーを報告する。
void expect(char *op)
{
	if(token->kind != TK_RESERVED || strlen(op) != token->len ||
		memcmp(token->str, op, token->len)){
		error_at(token->str, "'%s'ではありません", op);
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

//cがa~zもしくは A~Zかどうか
bool is_alpha(char c)
{
	if(('a' <= c && c <= 'z')||('A' <= c && c <= 'Z') || c == '_'){
		return true;
	}
	return false;
}

//cがアルファベットか数字ならばtrueを返す
bool is_alnum(char c)
{
	if(is_alpha(c) || ('0' <= c &&c <= '9')){
		return true;
	}
	return false;
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

		//return文の判定
		if(strncmp(p, "return", 6) == 0 && !is_alnum(p[6])){
			cur = new_token(TK_RETURN, cur, p, 6);
			p += 6;
			continue;
		}

		//ローカル変数
		if(is_alpha(*p)){
			char *q = p;
			p++;
			while(is_alnum(*p)){
				p++;
			}
			cur = new_token(TK_IDENT, cur, q, p - q);
			continue;
		}
		
		//複数文字
		if(startswith(p, "==") || startswith(p, "!=") ||
			startswith(p, ">=") || startswith(p, "<=")){
			cur = new_token(TK_RESERVED, cur, p, 2);
			p += 2;
			continue;
		}

		if(strchr("+-*/()<>;=", *p)){
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

		error_at(p, "invalid token!");
	}
	new_token(TK_EOF, cur, p, 0);
	return head.next;
}


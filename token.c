#include "1cc.h"

//Input string

char *current_input;

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

void verror_at(char *loc, char *fmt, va_list ap)
{
	int pos = loc - current_input;
	fprintf(stderr, "%s\n", current_input);
	fprintf(stderr, "%*s", pos, " ");
	fprintf(stderr, "^");
	vfprintf(stderr, fmt, ap);
	fprintf(stderr, "\n");
	exit(1);
}

void error_at(char *loc, char *fmt, ...)
{
	va_list ap;
	va_start(ap, fmt);
	verror_at(loc, fmt, ap);
}

void error_tok(Token *tok, char *fmt, ...)
{
	va_list ap;
	va_start(ap, fmt);
	verror_at(tok->loc, fmt, ap);
}

//指定されたトークンと文字列opを比べる
bool equal(Token *tok, char *op)
{
	return strlen(op) == tok->len &&
		!strncmp(tok->loc, op, tok->len);
}

Token *skip(Token *tok, char *op)
{
	if(!equal(tok, op)){
		error_tok(tok, "expected '%s'", op);		
	}
	return tok->next;
}

//新しいトークンを生成してcurに繋げる
Token *new_token(Tokenkind kind, Token *cur, char *str, int len)
{
	Token *tok = calloc(1, sizeof(Token));
	tok->kind = kind;
	tok->loc = str;
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

bool is_it(char *op)
{
	if(token->kind != TK_RESERVED || strlen(op) != token->len ||
		memcmp(token->loc, op, token->len)){
		return false;
	}
	return true;
}

bool startswith(char *p, char *q)
{
	return memcmp(p, q, strlen(q)) == 0;//p==qでtrueを返す
}

//入力文字列pをトークナイズしてそれを返す
Token *tokenize(char *p)
{
	current_input = p;
	Token head = {};
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

		//if文の判定
		if(strncmp(p, "if", 2) == 0 && !is_alnum(p[2])){
			cur = new_token(TK_IF, cur, p, 2);
			p += 2;
			continue;
		}

		//else文の判定
		if(strncmp(p, "else", 4) == 0 && !is_alnum(p[4])){
			cur = new_token(TK_ELSE, cur, p, 4);
			p += 4;
			continue;
		}


		//while文の判定
		if(strncmp(p, "while", 5) == 0 && !is_alnum(p[5])){
			cur = new_token(TK_WHILE, cur, p, 5);
			p += 5;
			continue;
		}

		//for文の判定
		if(strncmp(p, "for", 3) == 0 && !is_alnum(p[3])){
			cur = new_token(TK_FOR, cur, p, 3);
			p += 3;
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

		if(strchr("+-*/()<>;={}", *p)){
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

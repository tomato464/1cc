#include "1cc.h"

//Input string

static char *current_input;

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

static void verror_at(char *loc, char *fmt, va_list ap)
{
	int pos = loc - current_input;
	fprintf(stderr, "%s\n", current_input);
	fprintf(stderr, "%*s", pos, " ");
	fprintf(stderr, "^");
	vfprintf(stderr, fmt, ap);
	fprintf(stderr, "\n");
	exit(1);
}

static void error_at(char *loc, char *fmt, ...)
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

bool consume(Token **rest, Token *tok, char *op)
{
	if(equal(tok, op)){
		*rest = tok->next;
		return true;
	}
	*rest = tok;
	return false;
}

//新しいトークンを生成してcurに繋げる
static Token *new_token(Tokenkind kind, Token *cur, char *str, int len)
{
	Token *tok = calloc(1, sizeof(Token));
	tok->kind = kind;
	tok->loc = str;
	tok->len = len;
	cur->next = tok;
	return tok;
}

//cがa~zもしくは A~Zかどうか
static bool is_alpha(char c)
{
	if(('a' <= c && c <= 'z')||('A' <= c && c <= 'Z') || c == '_'){
		return true;
	}
	return false;
}

//cがアルファベットか数字ならばtrueを返す
static bool is_alnum(char c)
{
	if(is_alpha(c) || ('0' <= c &&c <= '9')){
		return true;
	}
	return false;
}

static bool startswith(char *p, char *q)
{
	return memcmp(p, q, strlen(q)) == 0;//p==qでtrueを返す
}

static bool is_keyword(Token *tok)
{
	char *key[] = {"while", "if", "for", "return", "else", "int"};

	for(int i = 0; i < sizeof(key) / sizeof(*key); i++){
		if(equal(tok, key[i])){
			return true;
		}
	}
	return false;
}

//先頭を渡される
static void convert_keyword(Token *tok)
{
	for(Token *token = tok; token->kind != TK_EOF; token = token->next){
		if(token->kind == TK_IDENT && is_keyword(token)){
			token->kind = TK_RESERVED;
		}	
	}
	return;
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

		if(isdigit(*p)){
			cur = new_token(TK_NUM, cur, p, 0);//とりあえず0を入れておく
			char *q = p;
			cur->val = strtol(p, &p, 10);//pの値を更新
			cur->len = p - q;
			continue;
		}


		//ローカル変数とキーワードをどちらもTK_IDENTとして認識させる
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

		if(ispunct(*p)){
			cur = new_token(TK_RESERVED, cur, p, 1);
			p++;
			continue;
		}

		error_at(p, "invalid token!");
	}
	new_token(TK_EOF, cur, p, 0);
	convert_keyword(head.next);//ここでキーワードを確認して変換させる
	return head.next;
}

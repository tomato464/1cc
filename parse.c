#include "1cc.h"

LVar *locals;

static Type *typespec(Token **rest, Token *tok);
static Type *declarator(Token **rest, Token *tok, Type *ty);
static Node *declaration(Token **rest, Token *tok);
static Node *compound_stmt(Token **rest, Token *tok);
static Node *stmt(Token **rest, Token *tok);
static Node *expr(Token **rest, Token *tok);
static Node *assign(Token **rest, Token *tok);
static Node *equality(Token **rest, Token *tok);
static Node *relational(Token **rest, Token *tok);
static Node *add(Token **rest, Token *tok);
static Node *mul(Token **rest, Token *tok);
static Node *unary(Token **rest, Token *tok);
static Node *primary(Token **rest, Token *tok);


static Node *new_node(Nodekind kind, Token *tok)
{
	Node *node = calloc(1, sizeof(Node));
	node->kind = kind;
	node->tok = tok;
	return node;
}

static Node *new_binary(Nodekind kind, Node *lhs, Node *rhs, Token *tok)
{
	Node *node = new_node(kind, tok);
	node->lhs = lhs;
	node->rhs = rhs;
	return node;
}

static Node *new_unary(Nodekind kind, Node *expr, Token *tok)
{
	Node *node = new_node(kind, tok);
	node->lhs = expr;
	return node;
}

static Node *new_num(long val, Token *tok)
{
	Node *node = new_node(ND_NUM, tok);
	node->val = val;
	return node;
}

static char *get_ident(Token *tok)
{
	if(tok->kind != TK_IDENT){
		error_tok(tok, "expected an identifier");
	}
	return strndup(tok->loc, tok->len);
}

static long get_number(Token *tok)
{
	if(tok->kind != TK_NUM){
		error_tok(tok, "expected a number");
	}
	return tok->val;
	
}

//変数名を検索する。見つからなかった場合はNULLを返す。
static LVar *find_lvar(Token *tok)
{
	for(LVar *lvar = locals; lvar; lvar = lvar->next){
		if(strlen(lvar->name) == tok->len && !strncmp(tok->loc, lvar->name, tok->len)){
			return lvar;
		}
	}
	return NULL;
}

static Node *new_lvar_node(LVar *lvar, Token *tok)
{
	Node *node = new_node(ND_LVAR, tok);
	node->lvar = lvar;
	return node;
}

static LVar *new_lvar(char *name, Type *ty)
{
	LVar *lvar = calloc(1, sizeof(LVar));
	lvar->name = name;
	lvar->ty = ty;
	lvar->next = locals;
	locals = lvar;
	return lvar;
}

//　typespec = "int"
static Type *typespec(Token **rest, Token *tok)
{
	*rest = skip(tok, "int");
	return ty_int;
}

// type-suffix	= ( "(" func-params? ")" )?
// func-params	= param ( "," param)*
// param	= typespec declarator
static Type *type_suffix(Token **rest, Token *tok, Type *ty)
{
	if(equal(tok, "(")){
		tok = tok->next;

		Type head = {};
		Type *cur = &head;

		while(!equal(tok, ")")){
			if(cur != &head){
				tok = skip(tok, ",");
			}

			Type *basety = typespec(&tok, tok);
			Type *ty = declarator(&tok, tok, basety);
			cur->next = copy_type(ty);
			cur = cur->next;
		}

		ty = func_type(ty);
		ty->params = head.next;
		*rest = tok->next;
		return ty;

	}
	*rest = tok;
	return ty;

}

static Type *declarator(Token **rest, Token *tok, Type *ty)
{
	while(consume(&tok, tok, "*")){
		ty = pointer_to(ty);
	}

	if(tok->kind != TK_IDENT){
		error_tok(tok, "expected a variable name");
	}

	ty = type_suffix(rest, tok->next, ty);
	ty->name = tok;
	return ty;
}

static Node *declaration(Token **rest, Token *tok)
{
	Type *basety = typespec(&tok, tok);

	Node head = {};
	Node *cur = &head;
	int cnt = 0;

	while(!equal(tok, ";")){
		if(cnt++ > 0){
			tok = skip(tok, ",");	
		}
		Type *ty = declarator(&tok, tok, basety);
		LVar *lvar = new_lvar(get_ident(ty->name), ty);

		if(!equal(tok, "=")){
			continue;
		}

		Node *lhs = new_lvar_node(lvar, ty->name);
		Node *rhs = assign(&tok, tok->next);
		Node *node = new_binary(ND_ASSIGN, lhs, rhs, tok);
		cur->next = new_unary(ND_EXPR_STMT, node, tok);
		cur = cur->next;
	}
	Node *node = new_node(ND_BLOCK, tok);
	node->body = head.next;
	*rest = tok->next;
	return node;
}



static Function *funcdef(Token **rest, Token *tok)
{
	locals = NULL;
	Type *ty = typespec(&tok, tok);
	ty = declarator(&tok, tok, ty);

	Function *fn = calloc(1, sizeof(Function));
	fn->name = get_ident(ty->name);

	for(Type *t = ty->params; t; t = t->next){
		new_lvar(get_ident(t->name), t);
	}
	fn->params = locals;

	tok = skip(tok, "{");
	fn->node = compound_stmt(rest, tok)->body;
	fn->locals = locals;
	return fn;
}

//stmt = "return" expr ";"
//	|"if" "(" expr ")" stmt ( "else" stmt)?
//	|"for"  "(" expr?  ";" expr?  ";" expr ")" stmt
//	|"while" "(" expr  ")" stmt
//	|"{" compuund-stmt
//	|expr ";"
static Node *stmt(Token **rest, Token *tok)
{
	if(equal(tok, "return")){
		Token *start = tok;
		Node *lhs = expr(&tok, tok->next);
		Node *node = new_unary(ND_RETURN, lhs, start);
		*rest = skip(tok, ";");
		return node;
	}

	if(equal(tok, "{")){
		return compound_stmt(rest, tok->next);
	}

	if(equal(tok, "if")){
		Node *node = new_node(ND_IF, tok);
		tok = skip(tok->next, "(");
		node->cond = expr(&tok, tok);
		tok = skip(tok, ")");
		node->then = stmt(&tok, tok);
		if(equal(tok, "else")){
			node->els = stmt(&tok, tok->next);
		}
		*rest = tok;
		return node;
	}

	if(equal(tok, "for")){
		Node *node = new_node(ND_FOR, tok);
		tok = skip(tok->next, "(");

		if(!equal(tok, ";")){
			node->init = expr(&tok, tok);
		}
		tok = skip(tok, ";");

		if(!equal(tok, ";")){
			node->cond = expr(&tok, tok);
		}
		tok = skip(tok, ";");

		if(!equal(tok, ")")){
			node->inc = expr(&tok, tok);
		}
		tok = skip(tok, ")");

		node->then = stmt(rest, tok);
		return node;
	}

	if(equal(tok, "while")){
		Node *node = new_node(ND_WHILE, tok);
		tok = skip(tok->next, "(");
		node->cond = expr(&tok, tok);
		tok = skip(tok, ")");
		node->then = stmt(rest, tok);
		return node;
	}
	
	Node *node = new_unary(ND_EXPR_STMT, expr(&tok, tok), tok);//lhsにしかついていない
	*rest = skip(tok, ";");
	return node;
}

static Node *compound_stmt(Token **rest, Token *tok)
{
	Node *node = new_node(ND_BLOCK, tok);

	Node head = {};
	Node *cur = &head;
	while(!equal(tok, "}")){
		if(equal(tok, "int")){
			cur->next = declaration(&tok, tok);
			cur = cur->next;
		}else{
			cur->next = stmt(&tok, tok);
			cur = cur->next;
		}
		//add_type(cur);
	}
	node->body = head.next;
	*rest = tok->next;
	return node;
}

static Node *expr(Token **rest, Token *tok)
{
	return assign(rest, tok);
}

static Node *assign(Token **rest, Token *tok)
{
	Node *node = equality(&tok, tok);

	
	if(equal(tok, "=")){
		Token *start = tok;
		Node *rhs = assign(&tok, tok->next);
		node = new_binary(ND_ASSIGN, node, rhs, start);
	}
	*rest = tok;
	return node;
}

//equality = relational ("=="relational | "!="relational)*
static Node *equality(Token **rest, Token *tok)
{
	Node *node = relational(&tok, tok);
	
	for(;;){
		Token *start = tok;
		if(equal(tok, "==")){
			Node *rhs = relational(&tok, tok->next);
			node = new_binary(ND_EQ, node, rhs, start);
			continue;
		}

		if(equal(tok, "!=")){
			Node *rhs = relational(&tok, tok->next);
			node = new_binary(ND_NE, node, rhs, start);
			continue;
		}

		*rest = tok;
		return node;
	}
}

//relational = add ("<" add | "<=" add | ">" add | ">=" add)*
static Node *relational(Token **rest, Token *tok)
{
	Node *node = add(&tok, tok);

	for(;;){
		Token *start = tok;
		if(equal(tok, "<")){
			Node *rhs = add(&tok, tok->next);
			node = new_binary(ND_LT, node, rhs, start);
			continue;
		}

		if(equal(tok, ">")){
			Node *rhs = add(&tok, tok->next);
			node = new_binary(ND_LT, rhs, node, start);
			continue;
		}
		if(equal(tok, "<=")){
			Node *rhs = add(&tok, tok->next);
			node = new_binary(ND_LE, node, rhs, start);
			continue;
		}
		if(equal(tok, ">=")){
			Node *rhs = add(&tok, tok->next);
			node = new_binary(ND_LE, rhs, node, start);
			continue;
		}
		*rest = tok;
		return node;
	}
}

// Cでは、＋はポインター型に対してうまく機能しない
// もし、pがポインターならp+nは n を追加するのではなく sizeof(*p)*n をp	の値に加算することになる
// 従って、p+n　n個の要素分先の番地を指すことになる
// 言い換えれば、加算をする前に整数値をスケーリングする必要がある。
// この関数はそれを解決している
static Node *new_add(Node *lhs, Node *rhs, Token *tok)
{
	add_type(lhs);
	add_type(rhs);
	
	// num + num
	if(is_integer(lhs->ty) && is_integer(rhs->ty)){
		return new_binary(ND_ADD, lhs, rhs, tok);
	}

	if(lhs->ty->base && rhs->ty->base){
		error_tok(tok, "invalid operands");
	}

	//Canonicalize num + ptr to ptr + num
	if(!lhs->ty->base && rhs->ty->base){
		Node *tmp = lhs;
		lhs = rhs;
		rhs = tmp;
	}

	// ptr + num
	rhs = new_binary(ND_MUL, rhs, new_num(8, tok), tok);
	return new_binary(ND_ADD, lhs, rhs, tok);
}

static Node *new_sub(Node *lhs, Node *rhs, Token *tok)
{
	add_type(lhs);
	add_type(rhs);

	//num - num
	if(is_integer(lhs->ty) && is_integer(rhs->ty)){
		return new_binary(ND_SUB, lhs, rhs, tok);
	}

	//ptr - num
	if(lhs->ty->base && is_integer(rhs->ty)){
		rhs = new_binary(ND_MUL, rhs, new_num(8, tok), tok);
		return new_binary(ND_SUB, lhs, rhs, tok);
	}

	// ptr - ptr, この二つの間にいくつの要素があるのかを返す
	if(lhs->ty->base && rhs->ty->base){
		Node *node = new_binary(ND_SUB, lhs, rhs, tok);
		return new_binary(ND_DIV, node, new_num(8, tok), tok);
	}

	error_tok(tok, "invalid operands");
}

// add = mul ("+" mul | "-"mul)*
static Node *add(Token **rest, Token *tok)
{
	Node *node = mul(&tok, tok);
	
	for(;;){
		Token *start = tok;

		if(equal(tok, "+")){
			Node *rhs = mul(&tok, tok->next);
			node = new_add(node, rhs, start);
			continue;
		}

		if(equal(tok, "-")){
			Node *rhs = mul(&tok, tok->next);
			node = new_sub(node, rhs, start);
			continue;
		}
		*rest = tok;
		return node;
	}
}

//mul = unary ("*"unary | "/"unary)*
static Node *mul(Token **rest, Token *tok)
{
	Node *node = unary(&tok, tok);

	for(;;){
		Token *start = tok;

		if(equal(tok, "*")){
			Node *rhs = unary(&tok, tok->next);
			node = new_binary(ND_MUL, node, rhs, start);
			continue;
		}

		if(equal(tok, "/")){
			Node *rhs = unary(&tok, tok->next);
			node = new_binary(ND_DIV, node, rhs, start);
			continue;
		}

		*rest = tok;
		return node;
	}
}

//unary = ('+' | '-')?primary
static Node *unary(Token **rest, Token *tok)
{
	if(equal(tok, "+")){
		return unary(rest, tok->next);
	}

	if(equal(tok, "-")){
		return new_binary(ND_SUB, new_num(0, tok), unary(rest, tok->next), tok); 
	}

	if(equal(tok, "&")){
		Node *lhs = unary(rest, tok->next);
		return new_unary(ND_ADDR, lhs, tok);
	}

	if(equal(tok, "*")){
		Node *lhs = unary(rest, tok->next);
		return new_unary(ND_DEREF, lhs, tok);
	}

	return primary(rest, tok);
}

//primary = num | "("expr")"| ident args?
// args = "(" ")"
static Node *primary(Token **rest, Token *tok)
{
	if(equal(tok, "(")){
		Node *node = expr(&tok, tok->next);
		*rest = skip(tok, ")");
		return node;
	}

	
	if(tok->kind == TK_IDENT){
		//function call
		if(equal(tok->next, "(")){
			Token *start = tok;
			tok = tok->next->next;

			Node head = {};
			Node *cur = &head;
			while(!equal(tok, ")")){
				if(cur != &head){
					tok = skip(tok, ",");
				}

				cur->next = assign(&tok, tok);
				cur = cur->next;
			}

			*rest = skip(tok, ")");

			Node *node = new_node(ND_FUNCALL, start);
			node->funcname = strndup(start->loc, start->len);
			node->args = head.next;
			return node;
		}

		//variable

		LVar *lvar = find_lvar(tok);
		if(!lvar){//無かったら
			error_tok(tok, "undefined variable");
		}
		*rest = tok->next;
		return new_lvar_node(lvar, tok);
	}

	Node *node = new_num(get_number(tok), tok);
	*rest = tok->next;
	return node;
}
///program = funcdef*
Function *parse(Token *tok)
{
	Function head = {};
	Function *cur = &head;
	while(tok->kind != TK_EOF){
		cur->next = funcdef(&tok, tok);
		cur = cur->next;
	}
	return head.next;
}

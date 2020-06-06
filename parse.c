#include "1cc.h"

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
	for(LVar *var = locals; var; var = var->next){
		if(var->len == tok->len && !memcmp(tok->loc, var->name, var->len)){
			return var;
		}
	}
	return NULL;
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
	Node head = {};
	Node *cur = &head;
	while(!equal(tok, "}")){
		cur->next = stmt(&tok, tok);
		cur = cur->next;
	}
	Node *node = new_node(ND_BLOCK, tok);
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

// add = mul ("+" mul | "-"mul)*
static Node *add(Token **rest, Token *tok)
{
	Node *node = mul(&tok, tok);
	
	for(;;){
		Token *start = tok;

		if(equal(tok, "+")){
			Node *rhs = mul(&tok, tok->next);
			node = new_binary(ND_ADD, node, rhs, start);
			continue;
		}

		if(equal(tok, "-")){
			Node *rhs = mul(&tok, tok->next);
			node = new_binary(ND_SUB, node, rhs, start);
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

		Node *node = calloc(1, sizeof(Node));
		node->kind = ND_LVAR;

		LVar *lvar = find_lvar(tok);
		if(lvar){
			node->offset = lvar->offset;	
		}
		else{
			lvar = calloc(1, sizeof(LVar));
			lvar->next = locals;
			lvar->name = tok->loc;
			lvar->len = tok->len;
			if(locals == NULL){
				lvar->offset = 8;
			}else{
				lvar->offset = locals->offset + 8;
			}
			node->offset = lvar->offset;
			locals = lvar;
		}
		*rest = tok->next;
		return node;
	}

	Node *node = new_num(get_number(tok), tok);
	*rest = tok->next;
	return node;
}
///program = stmt*
Function *parse(Token *tok)
{
	Function *prog = calloc(1, sizeof(Function));

	tok = skip(tok, "{");
	prog->node = compound_stmt(&tok, tok)->body;
	return prog; 
}

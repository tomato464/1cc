#include "1cc.h"
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

//変数名を検索する。見つからなかった場合はNULLを返す。
LVar *find_lvar(Token *tok)
{
	for(LVar *var = locals; var; var = var->next){
		if(var->len == tok->len && !memcmp(tok->str, var->name, var->len)){
			return var;
		}
	}
	return NULL;
}

void program(){
	int i = 0;
	while(!at_eof()){
		code[i] = stmt();
		i++;
	}
	code[i] = NULL;
}

Node *stmt()
{
	Node *node;

	if(consume_return()){
		node = calloc(1, sizeof(Node));//return文はネストされないからこうしてもいい
		node->kind = ND_RETURN;
		node->lhs = expr();
		consume(";");
		return node;
	}
	else if(consume_if()){
		node = calloc(1, sizeof(Node));
		node->kind = ND_IF;
		consume("(");
		node->lhs = expr();
		consume(")");
		node->rhs = calloc(1, sizeof(Node));
		node->rhs->lhs = stmt();
		if(consume_else()){
			node->rhs->kind = ND_ELSE;
			node->rhs->rhs = stmt();
		}
		return node;
	}
	else if(consume_while()){
		node = calloc(1, sizeof(Node));
		node->kind = ND_WHILE;
		consume("(");
		node->lhs = expr();
		consume(")");
		node->rhs = stmt();
		return node;
	}
	else{
		node  = expr();
	}

	if(!consume(";")){
		error("ここでミス");
	}

	return node;
}

Node *expr()
{
	return assign();
}

Node *assign()
{
	Node *node = equality();
	if(consume("=")){
		node = new_binary(ND_ASSIGN, node, assign());
	}
	else{
		return node;
	}
}

//equality = relational ("=="relational | "!="relational)*
Node *equality()
{
	Node *node = relational();
	
	for(;;){
		if(consume("==")){
			node = new_binary(ND_EQ, node, relational());
		}
		else if(consume("!=")){
			node = new_binary(ND_NE, node, relational());
		}
		else{
			return node;
		}
	}
}

//relational = add ("<" add | "<=" add | ">" add | ">=" add)*
Node *relational()
{
	Node *node = add();

	for(;;){
		if(consume("<")){
			node = new_binary(ND_LT, node, add());
		}
		else if(consume(">")){
			node = new_binary(ND_LT, add(), node);
		}
		else if(consume("<=")){
			node = new_binary(ND_LE, node, add());
		}
		else if(consume(">=")){
			node = new_binary(ND_LE, add(), node);
		}
		else{
			return node;
		}
	}
}

// add = mul ("+" mul | "-"mul)*
Node *add()
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

	Token *tok = consume_ident();
	if(tok){
		Node *node = calloc(1, sizeof(Node));
		node->kind = ND_LVAR;

		LVar *lvar = find_lvar(tok);
		if(lvar){
			node->offset = lvar->offset;	
		}
		else{
			lvar = calloc(1, sizeof(LVar));
			lvar->next = locals;
			lvar->name = tok->str;
			lvar->len = tok->len;
			if(locals == NULL){
				lvar->offset = 8;
			}else{
				lvar->offset = locals->offset + 8;
			}
			node->offset = lvar->offset;
			locals = lvar;
		}
		return node;
	}

	return new_num(expect_number());
}

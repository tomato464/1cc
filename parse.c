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
	Node *node = expr();
	expect(";");
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
		node = new_binary(ND_ASSIGN, node, equality());
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

	return new_num(expect_number());
}

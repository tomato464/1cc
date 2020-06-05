#include "1cc.h"

static int label = 1;
static char *argreg[] = {"rdi","rsi","rdx","rcx","r8","r9"};

//コードを生成
static void gen_lval(Node *node)
{
	if(node->kind != ND_LVAR){
		error("左辺地の値が変数ではありません。");
	}

	printf("	mov	rax,rbp\n");
	printf("	sub	rax,%d\n", node->offset);
	printf("	push	rax\n");
	return;
}

static void gen_expr(Node *node)
{
	switch (node->kind){
		case ND_NUM:
			printf("	push	%d\n", node->val);
			return;

		case ND_LVAR:
			gen_lval(node);
			printf("	pop	rax\n");
			printf("	mov	rax,[rax]\n");
			printf("	push	rax\n");
			return;

		case ND_ASSIGN:
			gen_lval(node->lhs);
			gen_expr(node->rhs);

			printf("	pop	rdi\n");
			printf("	pop	rax\n");
			printf("	mov	[rax],rdi\n");
			printf("	push	rdi\n");
			return;

		case ND_FUNCALL:{
			int nargs = 0;
			for(Node *arg = node->args; arg; arg = arg->next){
				gen_expr(arg);
				nargs++;
			}

			for(int i = 1; i <= nargs; i++){
				printf("	pop	rax\n");
				printf("	mov	%s,rax\n", argreg[nargs - i]);
			}

			printf("	mov	rax, 0\n");
			printf("	call	%s\n", node->funcname);
			printf("	push	rax\n");
			return;
		}
	}

	gen_expr(node->lhs);
	gen_expr(node->rhs);
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

		case ND_EQ:
			printf("	cmp	rax,rdi\n");
			printf("	sete	al\n");
			printf("	movzb	rax,al\n");
			break;

		case ND_NE:
			printf("	cmp	rax,rdi\n");
			printf("	setne	al\n");
			printf("	movzb	rax,al\n");
			break;

		case ND_LT:
			printf("	cmp	rax,rdi\n");
			printf("	setl	al\n");
			printf("	movzb	rax,al\n");
			break;

		case ND_LE:
			printf("	cmp	rax,rdi\n");
			printf("	setle	al\n");
			printf("	movzb	rax,al\n");
			break;

		default:
			error("invalid expression");
	}
	printf("	push	rax\n");
	return;

}

static void gen_stmt(Node *node)
{
	switch(node->kind){
		case ND_BLOCK:{
			for(Node *c = node->body; c; c = c->next){
				gen_stmt(c);
				printf("	pop	rax\n");
			}
			return;
		}

		case ND_IF:{
			int tmp = label++;
			label++;
			gen_expr(node->cond);
			printf("	pop	rax\n");
			printf("	cmp	rax,0\n");
			if(node->els){
				printf("je	.Lelse%d\n",tmp);
				gen_stmt(node->then);
				printf("jmp	.Lend%d\n", tmp);
				printf(".Lelse%d:\n", tmp);
				gen_stmt(node->els);
			}else{
				printf("	je	.Lend%d\n", tmp);
				gen_stmt(node->then);
			}
			printf(".Lend%d:\n", tmp);
			return;
		}

		case ND_FOR:{
			int tmp = label;
			label++;

			if(node->init){
				gen_expr(node->init);	
			}
			
			printf(".Lbegin%d:\n", tmp);
			if(node->cond){
				gen_expr(node->cond);
				printf("	pop	rax\n");
				printf("	cmp	rax,0\n");
				printf("	je	.Lend%d\n", tmp);
			}

			if(node->then){
				gen_stmt(node->then);
			}

			if(node->inc){
				gen_expr(node->inc);
			}
			
			printf("	jmp	.Lbegin%d\n", tmp);
			printf(".Lend%d:\n", tmp);
			return;
		}

		case ND_WHILE:{
			int tmp = label;
			label++;
			printf(".Lbegin%d:\n", tmp);
			gen_expr(node->cond);
			printf("	pop	rax\n");
			printf("	cmp	rax,0\n");
			printf("	je	.Lend%d\n", tmp);
			gen_stmt(node->then);
			printf("	jmp	.Lbegin%d\n", tmp);
			printf(".Lend%d:\n", tmp);
			return;
		}

		case ND_RETURN:{
			gen_expr(node->lhs);
			printf("	pop	rax\n");
			printf("	mov	rsp,rbp\n");
			printf("	pop	rbp\n");
			printf("	ret\n");
			return;
		}

		case ND_EXPR_STMT:{
			gen_expr(node->lhs);
			printf("	pop	rax\n");
			return;
		}

		default:{
			error("invalid statement");
		}
	}
}

void codegen(Node *node)
{
	
	//アセンブリの前半部分を出力
	printf(".intel_syntax noprefix\n");
	printf(".global main\n");
	printf("main:\n");

	//プロローグ
	//変数26個分の領域を確保する
	printf("	push	rbp\n");
	printf("	mov	rbp,rsp\n");
	printf("	sub	rsp,208\n");//26*8
	//先頭の式から順にコード生成
	for(Node *n = node; n; n = n->next){
		gen_stmt(n);
		printf("	pop	rax\n");
	}

	//エピローグ
	//最後の式の結果がRAXに残っているのでそれが返り値になる。
	printf("	mov	rsp,rbp\n");
	printf("	pop	rbp\n");
	printf("	ret\n");
}



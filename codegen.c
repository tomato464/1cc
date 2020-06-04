#include "1cc.h"
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

static int label = 1;
void gen(Node *node)
{
	int tmp;
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
			gen(node->rhs);

			printf("	pop	rdi\n");
			printf("	pop	rax\n");
			printf("	mov	[rax],rdi\n");
			printf("	push	rdi\n");
			return;

		case ND_BLOCK:
			for(Node *c = node->body; c; c = c->next){
				gen(c);
				printf("	pop	rax\n");
			}
			return;

		case ND_IF:
			tmp = label;
			label++;
			gen(node->cond);
			printf("	pop	rax\n");
			printf("	cmp	rax,0\n");
			if(node->els){
				printf("je	.Lelse%d\n",tmp);
				gen(node->then);
				printf("jmp	.Lend%d\n", tmp);
				printf(".Lelse%d:\n", tmp);
				gen(node->els);
			}else{
				printf("	je	.Lend%d\n", tmp);
				gen(node->then);
			}
			printf(".Lend%d:\n", tmp);
			return;

		case ND_FOR:
			tmp = label;
			label++;

			if(node->init){
				gen(node->init);	
			}
			
			printf(".Lbegin%d:\n", tmp);

			if(node->cond){
				gen(node->cond);
				printf("	pop	rax\n");
				printf("	cmp	rax,0\n");
				printf("	je	.Lend%d\n", tmp);
			}

			if(node->then){
				gen(node->then);
			}

			if(node->inc){
				gen(node->inc);
			}
			
			printf("	jmp	.Lbegin%d\n", tmp);
			printf(".Lend%d:\n", tmp);
			return;

		case ND_WHILE:
			tmp = label;
			label++;
			printf(".Lbegin%d:\n", tmp);
			gen(node->cond);
			printf("	pop	rax\n");
			printf("	cmp	rax,0\n");
			printf("	je	.Lend%d\n", tmp);
			gen(node->then);
			printf("	jmp	.Lbegin%d\n", tmp);
			printf(".Lend%d:\n", tmp);
			return;

		case ND_RETURN:
			gen(node->lhs);
			printf("	pop	rax\n");
			printf("	mov	rsp,rbp\n");
			printf("	pop	rbp\n");
			printf("	ret\n");
			return;

		case ND_FUNCALL:
			printf("	mov	rax, 0\n");
			printf("	call	%s\n", node->funcname);
			printf("	push	rax\n");
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
	}
	printf("	push	rax\n");
	return;
}

#include "1cc.h"

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


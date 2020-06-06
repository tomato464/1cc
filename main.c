#include "1cc.h"

int main(int argc, char **argv)
{
	if(argc != 2){
		error("%s: 引数が正しくありません。\n", argv[0]);
		return 1;
	}
	
	//トークナイズする
	Token *tok = tokenize(argv[1]);
	Function *prog = parse(tok);
	codegen(prog);

	return 0;
}

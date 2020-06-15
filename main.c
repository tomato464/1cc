#include "1cc.h"

static int align_to(int n, int align)
{
	return (n + align - 1) & ~(align - 1);
}

int main(int argc, char **argv)
{
	if(argc != 2){
		error("%s: 引数が正しくありません。\n", argv[0]);
	}
	
	//トークナイズする
	Token *tok = tokenize(argv[1]);
	Function *prog = parse(tok);

	// Assign offsets to local variables.
	for(Function *fn = prog; fn; fn = fn->next){
		int offset = 32; // 32 for callee-saved registers
		for(LVar *lvar = fn->locals; lvar; lvar = lvar->next){
			offset += lvar->ty->size;
			lvar->offset = offset;
		}
		fn->stack_size = align_to(offset, 16);
		
	}
	codegen(prog);
	return 0;
}

#include "syntaxTree.h"
#include "hashTable.h"
#include "semantic.h"
#include "ICtranslation.h"

extern void yyrestart(FILE *);
extern int yyparse();
extern int yylineo;
extern FILE* fout;

void printOC();

Node* root = NULL;
int errors = 0;


int main(int argc, char** argv){

    if (argc != 3) {
	printf("USE: \"./parser [TestFileName] [outputFileName]\" TO TEST!\n"); 
        return 1; 
    } 
    FILE* fp = fopen(argv[1], "r");
    if (!fp) {
        printf("Fail open file: argv[1]\n");
        return 1;
    }
    fout = fopen(argv[2], "w");  
    if (!fout) {
        printf("Fail open file: argv[2]\n");
        return 1;
    }

    yylineno = 1;
    yyrestart(fp);
    //exp1
    yyparse();

    if(errors == 0) {
		//exp2
        initHash();
		traverseTree(root, semanticAnalysis, "ExtDefList");
		checkHash();
	
		//printTree(root,0);

		//exp3
		traverseTree(root, InterCodeTranslation, "ExtDef");
		//printICList();
	
		//exp4
		printOC();
    }

    fclose(fout);

    return 0;
}


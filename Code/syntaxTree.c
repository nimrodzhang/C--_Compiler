#include "syntaxTree.h"
#include "semantic.h"


Node* newNode(char* symbol, char* text){
    Node* p = (Node*)malloc(sizeof(Node));
    strcpy(p->symbol, symbol);
    strcpy(p->text, text);
    p->lineno = yylineno;
    p->parent = NULL;
    for(int i=0; i<8; i++) {
        p->children[i] = NULL;
    }
    p->chnum = 0;
    
    return p;
}


Node* insertNode(char* symbol, int chnum, ...) {
    //get parameters
    va_list args;
    va_start(args, chnum);
    
    Node* p = (Node*)malloc(sizeof(Node));
    strcpy(p->symbol, symbol);
    for(int i=0; i<chnum; i++) {
        p->children[i] = va_arg(args, Node*);
    }
    p->lineno = p->children[0]->lineno;
    p->parent = NULL;
    p->chnum = chnum;
    va_end(args);

    return p;
}

void printTree(Node* p, int blank) {
    if(p == NULL) {
        return;
    }
    for(int i=0; i<blank; i++) {
        printf(" ");
    }
    if(p->chnum != 0) {
        printf("%s (%d)\n", p->symbol, p->lineno);
        for(int i=0; i<p->chnum; i++) {
            printTree(p->children[i], blank+2);
        }
    }
    else {
        if(strcmp(p->symbol, "INT") == 0) {
            printf("%s: %d\n", p->symbol, convert(p->text));
        }
        else if(strcmp(p->symbol, "FLOAT") == 0) {
            printf("%s: %f\n", p->symbol, atof(p->text));
        }
        else if(strcmp(p->symbol, "ID") == 0 || strcmp(p->symbol, "TYPE") == 0) {
            printf("%s: %s\n", p->symbol, p->text);
        }
        else {
            printf("%s\n", p->symbol);
        }
    }
}

int convert(char* text) {
    if(strlen(text)>1 && text[0] == '0') {
	if(text[1] == 'x' || text[1] == 'X') {
	    return (int)strtoul(text, 0, 16);
	}
	else {
	    return (int)strtoul(text, 0, 8);
	}
    }
    else {
	return atoi(text);
    }
}


void traverseTree(Node* p, void (*func)(Node *), char* entrance) {
	if(p == NULL) {
		return;
	}

	if(strcmp(p->symbol, entrance) == 0) {
		func(p);
		return;
	}

	if(p->chnum != 0) {
		for(int i=0; i<p->chnum; i++) {
			traverseTree(p->children[i], func, entrance);
		}
	}
}

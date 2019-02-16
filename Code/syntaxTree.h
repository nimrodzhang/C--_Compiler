#ifndef _SYNTAXTREE_H_
#define _SYNTAXTREE_H_

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <stdarg.h>

extern int yylineno;

typedef struct SyntaxTree {
    char symbol[32];  
    char text[32];
    int lineno;
    struct SyntaxTree* parent;
    struct SyntaxTree* children[8];
    int chnum;
}Node;

Node* newNode(char*, char*);
Node* insertNode(char*, int, ...);
void printTree(Node*, int);
int convert(char*);
void traverseTree(Node* p, void (*func)(Node *), char* entrance);

#endif

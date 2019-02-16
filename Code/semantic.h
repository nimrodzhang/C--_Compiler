#ifndef _SEMANTIC_H_
#define _SEMANTIC_H_

#include <stdbool.h>

#include "syntaxTree.h"

typedef struct FieldList_* FieldList;
typedef struct Type_* Type;

typedef struct Type_ {
	//kind used for define
	enum {BASIC, ARRAY, STRUCTURE} type_kind;
	union {
		enum {INT, FLOAT} basic;
		struct {
			Type elem;
			int size;
		}array;
		FieldList structure;
	};
}Type_;

typedef struct FieldList_ {
	char* name;
	Type type;
	FieldList tail;
}FieldList_;

void semanticAnalysis(Node* root);

bool fieldEqual(FieldList a, FieldList b);
bool typeEqual(Type a, Type b);

void ExtDefList(Node* p);
void ExtDef(Node* p);
void ExtDecList(Node* p, Type type);
Type Specifier(Node* p);
Type StructSpecifier(Node* p);
void VarDec(Node* p, Type type, FieldList fieldlist, int flag);
void FunDec(Node* p, Type type, int flag);
void VarList(Node* p, FieldList fieldlist, int flag);
void ParamDec(Node* p, FieldList fieldlist, int flag);
void CompSt(Node* p, Type type);
void StmtList(Node* p, Type type);
void Stmt(Node* p, Type type);
void DefList(Node* p, FieldList fieldlist, int flag);
void Def(Node* p, FieldList fieldlist, int flag);
void DecList(Node* p, Type type, FieldList fieldlist, int flag);
void Dec(Node* p, Type type, FieldList fieldlist, int flag);
Type Exp(Node* p);
bool Args(Node* p, FieldList fiedlist);

#endif

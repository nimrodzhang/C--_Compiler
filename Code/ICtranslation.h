#ifndef _ICTRANSLATION_H_
#define _ICTRANSLATION_H_

#include "IClist.h"

void InterCodeTranslation(Node* root);
void printICList();

void transExtDef(Node* p);
ICNode* transVarDec(Node* p);
ICNode* transFunDec(Node* p);
ICNode* transVarList(FieldList fieldlist);
ICNode* transCompSt(Node* p);
ICNode* transStmtList(Node* p);
ICNode* transStmt(Node* p);
ICNode* transDefList(Node* p);
ICNode* transDecList(Node* p);
ICNode* transDec(Node* p);
ICNode* transExp(Node* p, char* place);
ICNode* transArgs(Node* p, char **arg_list, int* flag);
ICNode* transCondition(Node* p, char* label1, char* label2);
ICNode* transAddress(Node* p, char* place);




#endif

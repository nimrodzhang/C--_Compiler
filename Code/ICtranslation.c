#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <stdio.h>
#include "hashTable.h"
#include "semantic.h"
#include "ICtranslation.h"

ICNode * ICroot = NULL;

static int func_cnt = 0;
static int t_no = 0;
static int label_no = 0;

static char *new_t() {
	char* str = (char*)malloc(sizeof(char)*10);
	sprintf(str, "t%d", t_no);
	t_no++;
	return str;
}

static char *new_label(){
	char *str = (char *)malloc(sizeof(char)*10);
	sprintf(str,"label%d",label_no);
	label_no++;
	return str;
}

static int typeSize(Type type) {
	if(type->type_kind == BASIC) {
		return 4;
	}
	else if(type->type_kind == ARRAY) {
		return type->array.size * typeSize(type->array.elem);
	}
	else if(type->type_kind == STRUCTURE) {
		int size = 0;
		FieldList fl = type->structure;
		while(fl != NULL) {
			size = size + typeSize(fl->type);
			fl = fl->tail;
		}
		return size;
	}
	else {
		printf("invalid type kind\n");
		assert(0);
	}
}

static Type getNodeType(Node* p) {
	if(strcmp(p->children[0]->symbol, "ID") == 0) {
		char* name = p->children[0]->text;
		HashNode* hp = searchHash(name, Variable);
		if(hp == NULL) {
			return NULL;
		}
		return hp->data.type;
	}
	else if(strcmp(p->children[1]->symbol, "DOT") == 0) {
		FieldList fl = getNodeType(p->children[0])->structure;
		while(strcmp(fl->name, p->children[2]->text) != 0) {
			fl = fl->tail;
		}
		return fl->type;
	}
	else {
		return getNodeType(p->children[0])->array.elem;
	}
}

void InterCodeTranslation(Node* p) {
	//printf("ICtrans begin\n");
	transExtDef(p);
}

void printICList() {
	ICNode* p = ICroot;
	int k = 0;
	while(1) {
		printIC(p);
		p = p->next;
		k++;
		if(p == ICroot) {
			break;
		}
	}
	//printf("ic num %d\n", k);
}

//ExtDefList
//ExtDef
void transExtDef(Node* p) {
	//printf("%d transExtDef begin\n", func_cnt);
	func_cnt++;
	if(p == NULL)
		return;
	//ExtDef -> Specifier FunDec CompSt
	if(strcmp(p->children[1]->symbol, "FunDec") == 0) {
		//printf("fundec\n");
		if(strcmp(p->children[2]->symbol, "CompSt") == 0) {
			//printf("CompSt\n");
			ICNode* node1 = transFunDec(p->children[1]);
			ICNode* node2 = transCompSt(p->children[2]);
			ICNode* node3 = concatIC(2, node1, node2);
			ICroot = concatIC(2, ICroot, node3);
		}
		else {
			//Specifier FunDec SEMI
			assert(0);
		}
	}
	else {
	printf("end nowhere\n");
	}
}

//ExtDecList
//Specifier
//StructSpecifier
//VarDec
ICNode* transVarDec(Node* p) {
	//printf("%d transVarDec begin\n", func_cnt);
	func_cnt++;

	while(strcmp(p->children[0]->symbol, "ID") != 0) {
		p = p->children[0];
		
	}
	char* name = p->children[0]->text;
	HashNode* hp = searchHash(name, Variable);
	if(hp == NULL) {
		return NULL;
	}
	else {	
		Type type = hp->data.type;
		if(type->type_kind == BASIC) {
			insertOp(name, VARIABLE);
			return NULL;
		}
		else {
			int size = typeSize(type);
			return newICNode(DEC, name, size);
		}
	}
}

//FunDec
ICNode* transFunDec(Node* p) {
	//printf("%d transFunDec begin\n", func_cnt);
	func_cnt++;
	char* name = p->children[0]->text;
	HashNode* hp = searchHash(name, Function);
	ICNode* node1 = newICNode(FUNC_DEC, name);
	ICNode* node2 = NULL;
	//FunDec -> ID LP VarList RP
	if(hp != NULL) {
		FieldList fl = hp->data.fieldlist;
		node2 = transVarList(fl->tail);
	}
	else {
		assert(0);
	}
	return concatIC(2, node1, node2);
}

//VarList
ICNode* transVarList(FieldList fieldlist) {
	//printf("%d transVarList begin\n", func_cnt);
	func_cnt++;
	ICNode* node = NULL;
	while(fieldlist != NULL) {
		node = concatIC(2, node, newICNode(PARAM, fieldlist));
		fieldlist = fieldlist->tail;
	}
	return node;
}
 
//ParamDec
//CompSt
ICNode* transCompSt(Node* p) {
	//printf("%d transCompSt begin\n", func_cnt);
	func_cnt++;
	//CompSt -> LC DefList StmtList RC
	ICNode* node1 = transDefList(p->children[1]);
	ICNode* node2 = transStmtList(p->children[2]);
	return concatIC(2, node1, node2);
}

//StmtList
ICNode* transStmtList(Node* p) {
	//printf("%d transStmtList begin\n", func_cnt);
	func_cnt++;
	//StmtList -> Stmt StmtList
	if(p == NULL) {
		return NULL;
	}
	ICNode* node1 = transStmt(p->children[0]);
	ICNode* node2 = transStmtList(p->children[1]);
	return concatIC(2, node1, node2);
}

//Stmt
ICNode* transStmt(Node* p) {
	//printf("%d transStmt begin\n", func_cnt);
	func_cnt++;
	//printf("stmt childnum %d\n", p->chnum);
	//Stmt -> CompSt
	if(p->chnum == 1) {
		return transCompSt(p->children[0]);
	}
	//Stmt -> Exp SEMI
	else if(p->chnum == 2) {
		return transExp(p->children[0], NULL);
	}
	//Stmt -> RETURN Exp SEMI
	else if(p->chnum == 3) {
		char* t = new_t();
		ICNode* node1 = transExp(p->children[1], t);
		ICNode* node2 = newICNode(RET, t);
		return concatIC(2, node1, node2);
	}
	//Stmt -> IF LP Exp RP Stmt %prec LOWER_THAN_ELSE
	//Stmt -> WHILE LP Exp RP Stmt
	else if(p->chnum == 5) {
		if(strcmp(p->children[0]->symbol, "IF") == 0) {
			char* label1 = new_label();
			char* label2 = new_label();
			ICNode* node1 = transCondition(p->children[2], label1, label2);
			ICNode* node2 = newICNode(LABEL, label1);
			ICNode* node3 = transStmt(p->children[4]);
			ICNode* node4 = newICNode(LABEL, label2);
			return concatIC(4, node1, node2, node3, node4);
		}
		else {
			char* label1 = new_label();
			char* label2 = new_label();
			char* label3 = new_label();
			ICNode* node1 = newICNode(LABEL, label1);
			ICNode* node2 = transCondition(p->children[2], label2, label3);
			ICNode* node3 = newICNode(LABEL, label2);
			ICNode* node4 = transStmt(p->children[4]);
			ICNode* node5 = newICNode(GOTO, label1);
			ICNode* node6 = newICNode(LABEL, label3);
			return concatIC(6, node1, node2, node3, node4, node5, node6);
		}
	}
	//Stmt -> IF LP Exp RP Stmt ELSE Stmt
	else if(p->chnum == 7) {
		char* label1 = new_label();
        char* label2 = new_label();
        char* label3 = new_label();
	    ICNode* node1 = transCondition(p->children[2], label1, label2);
        ICNode* node2 = newICNode(LABEL, label1);
        ICNode* node3 = transStmt(p->children[4]);
        ICNode* node4 = newICNode(GOTO, label3);
        ICNode* node5 = newICNode(LABEL, label2);
		ICNode* node6 = transStmt(p->children[6]);
		ICNode* node7 = newICNode(LABEL, label3);
        return concatIC(7, node1, node2, node3, node4, node5, node6, node7);
	}
	else {
		printf("Invalid Stmt choice\n");
		return NULL;
	}
}

//DefList
ICNode* transDefList(Node* p) {
	//printf("%d transDefList begin\n", func_cnt);
	func_cnt++;
	if(p == NULL) {
		return NULL;
	}
	//DefList -> Def DefList
	//Def -> Specifier DecList SEMI
	ICNode* node1 = transDecList(p->children[0]->children[1]);
	ICNode* node2 = transDefList(p->children[1]);
	return concatIC(2, node1, node2);
}

//Def
//no need

//DecList
ICNode* transDecList(Node* p) {
	//printf("%d transDecList begin\n", func_cnt);
	func_cnt++;
	//DecList -> Dec
	if(p->chnum == 1) {
		return transDec(p->children[0]);
	}
	//DecList -> Dec COMMA DecList
	else {
		ICNode* node1 = transDec(p->children[0]);
		ICNode* node2 = transDecList(p->children[2]);
		return concatIC(2, node1, node2);
	}
}

//Dec
ICNode* transDec(Node* p) {
	//printf("%d transDec begin\n", func_cnt);
	func_cnt++;
	//Dec -> VarDec
	if(p->chnum == 1) {
		return transVarDec(p->children[0]);
	}
	//Dec -> VarDec ASSIGNOP Exp
	else {
		for(Node* t = p->children[0]; t != NULL; t = t->children[0]) {
			if(strcmp(t->symbol, "ID") == 0) {
				return transExp(p->children[2], t->text);
			}
		}
		return NULL;
	}
}

//Exp
ICNode* transExp(Node* p, char* place) {
	//printf("%d transExp begin\n", func_cnt);
	func_cnt++;
	//Exp -> ID/INT/FLOAT
	if(p->chnum == 1) {
		return newICNode(ASSIGN, place, p->children[0]->text);
	}
	else if(p->chnum == 2) {
		//Exp -> MINUS Exp
		if(strcmp(p->children[0]->symbol, "MINUS") == 0) {
			char* t = new_t();
			ICNode* node1 = transExp(p->children[1], t);
			ICNode* node2 = newICNode(NEG, place, t);
			return concatIC(2, node1, node2);
		}
		//Exp -> NOT Exp
		else if(strcmp(p->children[0]->symbol, "NOT") == 0) {
			char* label1 = new_label();
            char* label2 = new_label();
            ICNode* node1 = newICNode(ASSIGN, place, "0");
            ICNode* node2 = transCondition(p, label1, label2);
            ICNode* node3 = newICNode(LABEL, label1);
            ICNode* node4 = newICNode(ASSIGN, place, "1");
            ICNode* node5 = newICNode(LABEL, label2);
            return concatIC(5, node1, node2, node3, node4, node5);
		}
	}
	else if(p->chnum == 3) {
		if(strcmp(p->children[2]->symbol, "Exp") == 0) {
			//Exp -> Exp ASSIGNOP Exp
			if(strcmp(p->children[1]->symbol, "ASSIGNOP") == 0) {
				char* t1 = new_t();
				char* t2 = new_t();
				ICNode* node1 = transExp(p->children[0], t1);
				ICNode* node2 = transExp(p->children[2], t2);
				ICNode* node3 = node1->prev;
				node1 = popIC(node1);
				node3->prev = node3->next = node3;
				if(node3->ic_kind == ASSIGN_STAR) {
					node3->ic_kind = STAR_ASSIGN;
				}
				node3->assign.left = node3->assign.right;
				node3->assign.right = searchOp(t2);
				ICNode* node4 = newICNode(ASSIGN, place, t2);
				return concatIC(4, node1, node2, node3, node4);
			}	
			//Exp -> Exp PLUS Exp
			else if(strcmp(p->children[1]->symbol, "PLUS") == 0) {
				char* t1 = new_t();
				char* t2 = new_t();
				ICNode* node1 = transExp(p->children[0], t1);
				ICNode* node2 = transExp(p->children[2], t2);
				ICNode* node3 = newICNode(ADD, place, t1, t2);
				return concatIC(3, node1, node2, node3);
			}
			//Exp -> Exp MINUS Exp
			else if(strcmp(p->children[1]->symbol, "MINUS") == 0) {
				char* t1 = new_t();
				char* t2 = new_t();
				ICNode* node1 = transExp(p->children[0], t1);
				ICNode* node2 = transExp(p->children[2], t2);
				ICNode* node3 = newICNode(SUB, place, t1, t2);
				return concatIC(3, node1, node2, node3);
			}
			//Exp -> Exp STAR Exp
			else if(strcmp(p->children[1]->symbol, "STAR") == 0) { 
				char* t1 = new_t();
				char* t2 = new_t();
				ICNode* node1 = transExp(p->children[0], t1);
				ICNode* node2 = transExp(p->children[2], t2);
				ICNode* node3 = newICNode(MUL, place, t1, t2);
				return concatIC(3, node1, node2, node3);
			}
			//Exp -> Exp DIV Exp
			else if(strcmp(p->children[1]->symbol, "DIV") == 0) {
				char* t1 = new_t();
				char* t2 = new_t();
				ICNode* node1 = transExp(p->children[0], t1);
				ICNode* node2 = transExp(p->children[2], t2);
				ICNode* node3 = newICNode(DIV, place, t1, t2);
				return concatIC(3, node1, node2, node3);
			}
			//Exp -> Exp AND Exp
			//Exp -> Exp OR Exp
			//Exp -> Exp RELOP Exp
			else if(strcmp(p->children[1]->symbol, "AND") == 0 || 
					strcmp(p->children[1]->symbol, "OR") == 0 ||
					strcmp(p->children[1]->symbol, "RELOP") == 0) {
				char* label1 = new_label();
				char* label2 = new_label();
				ICNode* node1 = newICNode(ASSIGN, place, "0");
				ICNode* node2 = transCondition(p, label1, label2);
				ICNode* node3 = newICNode(LABEL, label1);
				ICNode* node4 = newICNode(ASSIGN, place, "1");
				ICNode* node5 = newICNode(LABEL, label2);
				return concatIC(5, node1, node2, node3, node4, node5);
			}
		}
		//Exp -> LP Exp RP
		else if(strcmp(p->children[1]->symbol, "Exp") == 0) {
			return transExp(p->children[1], place);
		}
		//Exp -> Exp DOT ID
		else if(strcmp(p->children[1]->symbol, "DOT") == 0) {
			char* t = new_t();
			ICNode* node1 = transAddress(p, t);
			ICNode* node2 = newICNode(ASSIGN_STAR, place, t);
			return concatIC(2, node1, node2);
		}
		//Exp -> ID LP RP
		else if(strcmp(p->children[0]->symbol, "ID") == 0) {
			char* func = p->children[0]->text;
			if(strcmp(func, "read") == 0) {
				return newICNode(READ, place);
			}
			else {
				if(place != NULL) {
					return newICNode(FUNC_CALL, place, func);
				}
				else {
					return newICNode(FUNC_CALL, new_t(), func);
				}
			}
		}
	}
	else if(p->chnum == 4) {
		//Exp -> Exp LB Exp RB
		if(strcmp(p->children[0]->symbol, "Exp") == 0) {
			char* t = new_t();
			ICNode* node1 = transAddress(p, t);
			ICNode* node2 = newICNode(ASSIGN_STAR, place, t);
			return concatIC(2, node1, node2);
		}
		//Exp -> ID LP Args RP
		else if(strcmp(p->children[0]->symbol, "ID") == 0) {
			char* func = p->children[0]->text;
			int args = 0;

			HashNode* hp = searchHash(func, Function);
			if(hp == NULL) {
				return NULL;
			}
			
			FieldList fl = hp->data.fieldlist;
			while(fl->tail != NULL) {
				args++;
				fl = fl->tail;
			}
			assert(args >= 1);
			
			char **arg_list = (char **)malloc(sizeof(char*) * args);
			int* flag = (int*)malloc(sizeof(int) * args);
			fl = (hp->data.fieldlist)->tail;
			int i = 0;
			while(fl != NULL) {
				if(fl->type != NULL && (fl->type->type_kind == STRUCTURE || (fl->type->type_kind == ARRAY && fl->type->array.elem != NULL))) {
					flag[i] = 1;
				}
				else {
					flag[i] = 0;
				}
				i++;
				fl = fl->tail;
			}
			assert(i == args);
			
			ICNode* node1 = transArgs(p->children[2], arg_list, flag);
			ICNode* node2 = NULL;
			if(strcmp(func, "write") == 0) {
				node2 = newICNode(WRITE, arg_list[0]);
				return concatIC(2, node1, node2);
			}
			else {
				for(int i=args-1; i>=0; i--) {
					node2 = concatIC(2, node2, newICNode(ARG, arg_list[i]));
				}
				ICNode* node3 = NULL;
				if(place != NULL) {
					node3 = newICNode(FUNC_CALL, place, func);
				}
				else {
					node3 = newICNode(FUNC_CALL, new_t(), func);
				}
				return concatIC(3, node1, node2, node3);
			}
		}
	}
	
	return NULL;
}

//Args
ICNode* transArgs(Node* p, char **arg_list, int* flag) {
	//printf("%d transArgs begin\n", func_cnt);
	func_cnt++;
	char* t = new_t();
	ICNode* node1 = transExp(p->children[0], t);
	Operand op = searchOp(t);
	if(flag[0] && op->op_kind == VARIABLE) {
		ICNode* temp = node1->prev;
		temp->ic_kind = ASSIGN_ADDR;
		temp->assign.left->op_kind = ADDRESS;
	}
	*arg_list = t;
	//Args -> Exp
	if(p->chnum == 1) {
		return node1;
	}
	//Args -> Exp COMMA Args
	else {
		ICNode* node2 = transArgs(p->children[2], arg_list+1, flag+1);
		return concatIC(2, node1, node2);
	}
}

//Condition
ICNode* transCondition(Node* p, char* label1, char* label2) {
	//printf("%d transCondition begin\n", func_cnt);
	func_cnt++;
	if(p->chnum == 2) {
		//Exp -> NOT Exp
		if(strcmp(p->children[0]->symbol, "NOT") == 0) {
			return transCondition(p->children[1], label2, label1);
		}
	}
	if(p->chnum == 3) {
		//Exp -> Exp AND Exp
		//	  -> Exp OR Exp
		if(strcmp(p->children[1]->symbol, "AND") == 0 ||
			strcmp(p->children[1]->symbol, "OR") == 0) {
			char* label = new_label();
			ICNode* node1 = NULL;
			if(strcmp(p->children[1]->symbol, "AND") == 0) {
				node1 = transCondition(p->children[0], label, label2);
			}
			else {
				node1 = transCondition(p->children[0], label1, label);
			}
			ICNode* node2 = newICNode(LABEL, label);
			ICNode* node3 = transCondition(p->children[2], label1, label2);
			return concatIC(3, node1, node2, node3);
		}
		else if(strcmp(p->children[1]->symbol, "RELOP") == 0) {
			char* t1 = new_t();
			char* t2 = new_t();
			char* op = p->children[1]->text;
			ICNode* node1 = transExp(p->children[0], t1);
			ICNode* node2 = transExp(p->children[2], t2);
			ICNode* node3 = newICNode(IF, t1, t2, op, label1);
			//ICNode* node4 = newICNode(GOTO, label1);
			ICNode* node4 = newICNode(GOTO, label2);
			return concatIC(4, node1, node2, node3, node4);
		}
	}

	//other circumstances like
	//when !(Exp) or !Exp become (Exp) or Exp
	char* t = new_t();
	ICNode* node1 = transExp(p, t);
	ICNode* node2 = newICNode(IF, t, "0", "!=", label1);
	//ICNode* node3 = newICNode(GOTO, label1);
	ICNode* node3 = newICNode(GOTO, label2);
	return concatIC(3, node1, node2, node3);
}

//Address
ICNode* transAddress(Node* p, char* place) {
	//printf("%d transAddress begin\n", func_cnt);
	func_cnt++;
	if(p->chnum == 1) {
		char* name = p->children[0]->text;
		Operand op = searchOp(name);
		if(op->op_kind == VARIABLE) {
			return newICNode(ASSIGN_ADDR, place, name);
		}
		else if(op->op_kind == ADDRESS) {
			return newICNode(ASSIGN, place, name);
		}
		else {
			//can't be constant
			return NULL;
		}
	}
	//Exp -> Exp DOT ID
	else if(p->chnum == 3) {
		char* t = new_t();
		ICNode* node1 = transAddress(p->children[0], t);
		FieldList fl = getNodeType(p->children[0])->structure;
		int size = 0;
		while(strcmp(fl->name, p->children[2]->text) != 0) {
			size = size + typeSize(fl->type);
			fl = fl->tail;
		}
		char* str = (char*)malloc(sizeof(char) * 10);
		sprintf(str, "%d", size);
		ICNode* node2 = newICNode(ADD, place, t, str);
		return concatIC(2, node1, node2);
	}
	//Exp -> Exp [ Exp ]
	else if(p->chnum == 4) {
		char* t1 = new_t();
		char* t2 = new_t();
		char* t3 = new_t();
		int size = typeSize(getNodeType(p));
		char* str = (char*)malloc(sizeof(char) * 10);
		sprintf(str, "%d", size);
		ICNode* node1 = transAddress(p->children[0], t1);
		ICNode* node2 = transExp(p->children[2], t2);
		ICNode* node3 = newICNode(MUL, t3, t2, str);
		ICNode* node4 = newICNode(ADD, place, t1, t3);
		return concatIC(4, node1, node2, node3, node4);
	}
	else {
		return NULL;
	}
}


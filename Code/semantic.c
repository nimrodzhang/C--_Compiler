#include <assert.h>

#include "semantic.h"
#include "hashTable.h"


bool fieldEqual(FieldList a, FieldList b) {
	//both NULL
	if(!a && !b) {
		return true;
	}
	//one NULL
	if((a && !b) || (!a && b)) {
		return false;
	}
	return typeEqual(a->type, b->type) && fieldEqual(a->tail, b->tail);
}

bool typeEqual(Type a, Type b) {
	//both NULL
	if(!a && !b) {
		return true;
	}
	//one NULL
	if((a && !b) || (!a && b)) {
	    return false;
	}
	if(a->type_kind == b->type_kind) {
		if(a->type_kind == BASIC) {
			return (a->basic == b->basic);
		}
		if(a->type_kind == ARRAY) {
			return typeEqual(a->array.elem, b->array.elem);
		}
		if(a->type_kind == STRUCTURE) {
			return fieldEqual(a->structure, b->structure);
		}
	}
	return false;
}

void semanticAnalysis(Node* root) {
	ExtDefList(root);
}

void ExtDefList(Node* p) {
	if(p == NULL) {
		return;
	}
	//ExtDefList -> ExtDef ExtDefList
	if(p->chnum == 2) {
		ExtDef(p->children[0]);
		ExtDefList(p->children[1]);
	}
	else {
		assert(0);
	}
}

void ExtDef(Node* p) {
	Type type = Specifier(p->children[0]);
	//ExtDef -> Specifier ExtDecList SEMI
	if(strcmp(p->children[1]->symbol, "ExtDecList") == 0) {
		if(type == NULL) {
			printf("Error type 17 at Line %d: Undefined structure.\n", p->lineno);
		}
		else {
			ExtDecList(p->children[1], type);
		}
	}
	//ExtDef -> Specifier FunDec CompSt
	//ExtDef -> Specifier FunDec SEMI
	else if(strcmp(p->children[1]->symbol, "FunDec") == 0) {
		if(type == NULL) {
			printf("Error type 17 at Line %d: Undefined structure.\n", p->lineno);
		}
		if(strcmp(p->children[2]->symbol, "CompSt") == 0) {
			FunDec(p->children[1], type, 1);
			CompSt(p->children[2], type);
		}
		else if(strcmp(p->children[2]->symbol, "SEMI") == 0) {
			FunDec(p->children[1], type, 0);
		}
		else {
			assert(0);
		}
	}
	//ExtDef -> Specifier SEMI
	else if(strcmp(p->children[1]->symbol, "SEMI") == 0) {
		//do nothing
	}
	else {
		assert(0);
	}
}

void ExtDecList(Node* p, Type type) {
	//ExtDecList -> VarDec
	VarDec(p->children[0], type, NULL, 2);
	//ExtDecList -> VarDec COMMA ExtDecList
	if(p->chnum == 3) {
		//VarDec()
		ExtDecList(p->children[2], type);
	}
	else if(p->chnum != 1) {
		assert(0);
	}
}

Type Specifier(Node* p) {
	Type type = NULL;
	//Specifier -> TYPE
	if(strcmp(p->children[0]->symbol, "TYPE") == 0) {
		type = (Type)malloc(sizeof(Type_));
		type->type_kind = BASIC;
		if(strcmp(p->children[0]->text, "int") == 0) {
			type->basic = INT;
		}
		else if(strcmp(p->children[0]->text, "float") == 0) {
			type->basic = FLOAT;
		}
		else {
			assert(0);
		}
	}
	//Specifier -> StructSpecifier
	else if(strcmp(p->children[0]->symbol, "StructSpecifier") == 0) {
		type = StructSpecifier(p->children[0]);
	}
	else {
		assert(0);
	}
	return type;
}

Type StructSpecifier(Node* p) {
	Type type = NULL;
	//StructSpecifier -> STRUCT Tag
	if(p->chnum == 2) {
		HashNode* hp = searchHash(p->children[1]->children[0]->text, Structure);
		//printf("search\n");	
		if(hp != NULL) {
			//printf("!null\n");
			FieldList fieldlist;
			memcpy(&fieldlist, &hp->data, sizeof(fieldlist));
			type = (Type)malloc(sizeof(Type_));
			type->type_kind = STRUCTURE;
			type->structure = fieldlist;
			//if(fieldlist == NULL)
			//	printf("f null\n");
		}
	}
	//StructSpecifier -> STRUCT OptTag LC DefList RC
	else if(p->chnum == 5) {
		type = (Type)malloc(sizeof(Type_));
		type->type_kind = STRUCTURE;
		type->structure = NULL;
	
		//3 must come before 1 so structure was filled before insert	
		if(p->children[3] != NULL) {
            FieldList fieldlist = (FieldList)malloc(sizeof(FieldList_));
            fieldlist->tail = NULL; 
            DefList(p->children[3], fieldlist, 1);
            type->structure = fieldlist->tail;
        }
		if(p->children[1] != NULL) {
			if(searchHash(p->children[1]->children[0]->text, Variable) != NULL || searchHash(p->children[1]->children[0]->text, Structure) != NULL) {
				printf("Error type 16 at Line %d: Duplicated name \"%s\".\n", p->lineno, p->children[1]->children[0]->text);
			}
			else {
				int ret = insertHash(p->children[1]->children[0]->text, Structure, type->structure, 0);
				assert(ret);
			}
		}
	}
	else {
		assert(0);
	}
	return type;

}

void VarDec(Node* p, Type type, FieldList fieldlist, int flag) {
	//flag	meaning
	//1		Structure(field)
	//2		Variable
	//3		both
	//VarDec -> ID
	if(p->chnum == 1) {
		//Variable
		if(flag&2) {
			if(searchHash(p->children[0]->text, Variable) != NULL || searchHash(p->children[0]->text, Structure) != NULL) {
				printf("Error type 3 at Line %d: Redefined variable \"%s\".\n", p->lineno, p->children[0]->text);
			}
			else {
				int ret = insertHash(p->children[0]->text, Variable, type, 0);
				assert(ret);
			}
		}
		//field
		if(flag&1) {
			FieldList fl = fieldlist;
			while(fl->tail != NULL) {
				if(strcmp(p->children[0]->text, fl->tail->name) == 0) {
					printf("Error type 15 at Line %d: Redefined field \"%s\".\n", p->lineno, p->children[0]->text);
					break;
				}
				fl = fl->tail;	
			}
			//fieldlist is originally from StructSpecifier
			//next 4 lines fill the field of a struct with vars
			//all struct will be empty if deleted
			fl->tail = (FieldList)malloc(sizeof(FieldList_));
			fl->tail->name = p->children[0]->text;
			fl->tail->type = type;
			fl->tail->tail = NULL;
		}
	}
	//VarDec -> VarDec LB INT RB
	else if(p->chnum == 4) {
		Type array = (Type)malloc(sizeof(Type_));
		array->type_kind = ARRAY;
		array->array.elem = type;
		array->array.size = convert(p->children[2]->text);
		VarDec(p->children[0], array, fieldlist, flag);
	}
	else {
		assert(0);
	}
}

void FunDec(Node* p, Type type, int flag) {
	//flag	meaning
	//0		declaration
	//1		definition
	FieldList fieldlist = (FieldList)malloc(sizeof(FieldList_));
	fieldlist->name = "return";
	fieldlist->type = type;
	fieldlist->tail = NULL;
	//FunDec -> ID LP RP
	if(p->chnum == 3) {
		//no arguments
	}
	//FunDec -> ID LP VarList RP
	else if(p->chnum == 4) {
		VarList(p->children[2], fieldlist, flag);
	}
	else {
		assert(0);
	}

	int lineno;
	if(flag == 0) {
		lineno = p->lineno;
	}
	else {
		lineno = 0;
	}

	HashNode* hp = searchHash(p->children[0]->text, Function);
	if(hp != NULL) {
		if(lineno == 0) {
			if(hp->lineno == 0) {
				printf("Error type 4 at Line %d: Redefined function \"%s\".\n", p->lineno, p->children[0]->text);
			}
			else {
				if(!fieldEqual(fieldlist, hp->data.fieldlist)) {
					printf("Error type 19 at Line %d: Inconsistent declaration of function \"%s\".\n", p->lineno, p->children[0]->text);
				}
				else {
					hp->lineno = 0;
				}
			}
		}
		else {
			if(hp->lineno == 0) {
				if(!fieldEqual(fieldlist, hp->data.fieldlist)) {
					printf("Error type 19 at Line %d: Inconsistent declaration of function \"%s\".\n", p->lineno, p->children[0]->text);
				}
			}
			else {
				if(!fieldEqual(fieldlist, hp->data.fieldlist)) {
					printf("Error type 19 at Line %d: Inconsistent declaration of function \"%s\".\n", p->lineno, p->children[0]->text);
				}
				else {
					hp->lineno = lineno;
				}
			}
		}
	}
	else {
		int ret = insertHash(p->children[0]->text, Function, fieldlist, lineno);
		assert(ret);
	}
}


void VarList(Node* p, FieldList fieldlist, int flag) {
	//VarList -> ParamDec
	ParamDec(p->children[0], fieldlist, flag);
	//VarList -> ParamDec COMMA VarList
	if(p->chnum == 3) {
		//ParamDec()
		VarList(p->children[2], fieldlist, flag);
	}
	else if(p->chnum != 1) {
		assert(0);
	}
}

void ParamDec(Node* p, FieldList fieldlist, int flag) {
//printf("%s  %s\n", p->symbol, p->text);
	//ParamDec -> Specifier VarDec
	Type type = Specifier(p->children[0]);
	if(type == NULL) {
		printf("Error type 17 at Line %d: Undefined structure.\n", p->lineno);
	}
	else {
		if(flag) {
			VarDec(p->children[1], type, fieldlist, 3);
		}
		else {
			VarDec(p->children[1], type, fieldlist, 1);
		}
	}
}

void CompSt(Node* p, Type type) {
	//CompSt -> LC DefList StmtList RC
	DefList(p->children[1], NULL, 2);
	StmtList(p->children[2], type);
}

void StmtList(Node* p, Type type) {
	if(p == NULL) {
		return;
	}
	//StmtList -> Stmt StmtList
	if(p->chnum == 2) {
		Stmt(p->children[0], type);
		StmtList(p->children[1], type);
	}
	else {
		assert(0);
	}
}

void Stmt(Node* p, Type type) {
	Type exp;
	//Stmt -> CompSt
	if(p->chnum == 1) {
		CompSt(p->children[0], type);
	}
	//Stmt -> Exp SEMI
	else if(p->chnum == 2) {
		exp = Exp(p->children[0]);
	}
	//Stmt -> RETURN Exp SEMI
	else if(p->chnum == 3) {
		exp = Exp(p->children[1]);
		if(exp == NULL) {
			return;
		}
		if(!typeEqual(type, exp)) {
			printf("Error type 8 at Line %d: Type mismatched for return.\n", p->lineno);
		}
	}
	//Stmt -> IF LP Exp RP Stmt %prec LOWER_THAN_ELSE
	//Stmt -> WHILE LP Exp RP Stmt
	else if(p->chnum == 5) {
		exp = Exp(p->children[2]);
		if(exp == NULL) {
			return;
		}
		if(!(exp->type_kind == BASIC && exp->basic == INT)) {
			printf("Error type 7 at Line %d: Type mismatched for operands.\n", p->lineno);
		}
		Stmt(p->children[4], type);
	}
	//Stmt -> IF LP Exp RP Stmt ELSE Stmt
	else if(p->chnum == 7) {
		exp = Exp(p->children[2]);
		if(exp == NULL) {
			return;
		}
		if(!(exp->type_kind == BASIC && exp->basic == INT)) {
			printf("Error type 7 at Line %d: Type mismatched for operands.\n", p->lineno);
		}
		Stmt(p->children[4], type);
		Stmt(p->children[6], type);
	}
	else {
		assert(0);
	}
}

void DefList(Node* p, FieldList fieldlist, int flag) {
	if(p == NULL) {
		return;
	}
	//DefList -> Def DefList
	if(p->chnum == 2) {
		Def(p->children[0], fieldlist, flag);
		DefList(p->children[1], fieldlist, flag);
	}
	else {
		assert(0);
	}
}

void Def(Node* p, FieldList fieldlist, int flag) {
	//Def -> Specifier DecList SEMI
	Type type = Specifier(p->children[0]);
	if(type == NULL) {
		printf("Error type 17 at Line %d: Undefined structure.\n", p->lineno);
	}
	else {
		DecList(p->children[1], type, fieldlist, flag);
	}
}

void DecList(Node* p, Type type, FieldList fieldlist, int flag) {
	//DecList -> Dec
	Dec(p->children[0], type, fieldlist, flag);
	//DecList -> Dec COMMA DecList
	if(p->chnum == 3) {
		//Dec()
		DecList(p->children[2], type, fieldlist, flag);
	}
	else if(p->chnum != 1) {
		assert(0);
	}
}

void Dec(Node* p, Type type, FieldList fieldlist, int flag) {
	//Dec -> VarDec
	if(p->chnum == 1) {
		VarDec(p->children[0], type, fieldlist, flag);
	}
	//Dec -> VarDec ASSIGNOP Exp
	if(p->chnum == 3) {
		VarDec(p->children[0], type, fieldlist, flag);
		if(flag == 1) {
			printf("Error type 15 at Line %d: Redefined field.\n", p->lineno);
		}
		else if(flag == 2) {
			if(!(strcmp(p->children[0]->children[0]->symbol, "ID") == 0 && typeEqual(type, Exp(p->children[2])))) {
				//Type tt = Exp(p->children[2]);
				//printf("%s %d\n", p->children[0]->children[0]->symbol, type->type_kind);
				printf("Error type 5 at Line %d: Type mismatched for assignment.\n", p->lineno);
			}
		}
	}
}

Type Exp(Node* p) {
	if(p->chnum == 1) {
		//Exp -> ID
		if(strcmp(p->children[0]->symbol, "ID") == 0) {
			HashNode* hp = searchHash(p->children[0]->text, Variable);
			if(hp == NULL) {
				printf("Error type 1 at Line %d: Undefined variable \"%s\".\n", p->lineno, p->children[0]->text);
			}
			else {
				Type type;
				memcpy(&type, &hp->data, sizeof(type));
				return type;
			}
		}
		//Exp -> INT
		else if(strcmp(p->children[0]->symbol, "INT") == 0) {
			Type type = (Type)malloc(sizeof(Type_));
			type->type_kind = BASIC;
			type->basic = INT;
			return type;
		}
		//Exp -> FLOAT
		else if(strcmp(p->children[0]->symbol, "FLOAT") == 0) {
			Type type = (Type)malloc(sizeof(Type_));
			type->type_kind = BASIC;
			type->basic = FLOAT;
			return type;
		}
		else {
			assert(0);
		}
	}
	else if(p->chnum == 2) {
		Type type = Exp(p->children[1]);
		//Exp -> MINUS Exp
		if(strcmp(p->children[0]->symbol, "MINUS") == 0) {
			if(type != NULL && type->type_kind == BASIC && (type->basic == INT || type->basic == FLOAT)) {
				return type;
			}
			else {
				printf("Error type 7 at Line %d: Type mismatched for operands.\n", p->lineno);
			}
		}
		//Exp -> NOT Exp
		else if(strcmp(p->children[0]->symbol, "NOT") == 0) {
			if(type != NULL && type->type_kind == BASIC && type->basic == INT) {
				return type;
			}
			else {
				printf("Error type 7 at Line %d: Type mismatched for operands.\n", p->lineno);
			}
		}
		else {
			assert(0);
		}
	}
	else if(p->chnum == 3) {
		if(strcmp(p->children[0]->symbol, "Exp") == 0 && strcmp(p->children[2]->symbol, "Exp") == 0) {
			Type type_1 = Exp(p->children[0]);
			Type type_2 = Exp(p->children[2]);
			if(type_1 == NULL || type_2 == NULL) {
				return NULL;
			}
			//Exp -> Exp ASSIGNOP Exp
			if(strcmp(p->children[1]->symbol, "ASSIGNOP") == 0) {
				//if(type_1->structure == NULL)
				//	printf("NULL");
				if(!typeEqual(type_1, type_2)) {
					printf("Error type 5 at Line %d: Type mismatched for assignment.\n", p->lineno);
				}
				//Exp -> ID = Exp
				//	  -> Exp.Exp = Exp
				//	  -> Exp[Exp] = Exp
				else if((p->children[0]->chnum == 1 && strcmp(p->children[0]->children[0]->symbol, "ID") == 0)
						|| (p->children[0]->chnum == 3 && strcmp(p->children[0]->children[1]->symbol, "DOT") == 0)
						|| (p->children[0]->chnum == 4 && strcmp(p->children[0]->children[1]->symbol, "LB") == 0)) {
					return type_1;
				}
				else {
					printf("Error type 6 at Line %d: The left-hand side of an assignment must be a variable.\n", p->lineno);
				}
			}
			//Exp -> Exp AND Exp
			//Exp -> Exp OR Exp
			else if(strcmp(p->children[1]->symbol, "AND") == 0 || strcmp(p->children[1]->symbol, "OR") == 0) {
				if(type_1->type_kind == BASIC && type_1->basic == INT && type_2->type_kind == BASIC && type_2->basic == INT) {
					return type_1;
				}
				else {
					printf("Error type 7 at Line %d: Type mismatched for operands.\n", p->lineno);
				}
			}
			/*
			//Exp -> Exp RELOP Exp
			else if(strcmp(p->children[1]->symbol, "RELOP") == 0) {
				if(typeEqual(type_1, type_2) && type_1->type_kind == BASIC) {
					return type_1;
				}
				else {
					printf("Error type 7 at Line %d: Type mismatched for operands.\n", p->lineno);
				}
			}
			*/
			//Exp -> Exp RELOP Exp
			//Exp -> Exp PLUS Exp
			//    -> Exp MINUS Exp
			//    -> Exp STAR Exp
			//    -> Exp DIV Exp
			else if(strcmp(p->children[1]->symbol, "RELOP") == 0
					|| strcmp(p->children[1]->symbol, "PLUS") == 0
					|| strcmp(p->children[1]->symbol, "MINUS") == 0
					|| strcmp(p->children[1]->symbol, "STAR") == 0
					|| strcmp(p->children[1]->symbol, "DIV") == 0) {
				//printf("enter plus\n");
				if(typeEqual(type_1, type_2) && type_1->type_kind == BASIC && (type_1->basic == INT || type_1->basic == FLOAT)) {
				//printf("enter if\n");
					return type_1;
				}
				else {
					printf("Error type 7 at Line %d: Type mismatched for operands.\n", p->lineno);
				}
			}
			else {
				assert(0);
			}
		}
		//Exp -> LP Exp RP
		else if(strcmp(p->children[1]->symbol, "Exp") == 0) {
			return Exp(p->children[1]);
		}
		//Exp -> Exp DOT ID
		else if(strcmp(p->children[1]->symbol, "DOT") == 0) {
			Type type = Exp(p->children[0]);
			if(type == NULL || type->type_kind != STRUCTURE) {
				printf("Error type 13 at Line %d: Illegal use of \".\".\n", p->lineno);
			}
			else {
				FieldList fl = type->structure;
				Type temp = NULL;
				while(fl != NULL) {
					if(strcmp(fl->name, p->children[2]->text) == 0) {
						temp = fl->type;
						break;
					}
					fl = fl->tail;
				}
				if(temp == NULL) {
					printf("Error type 14 at Line %d: Non-existent field \"%s\".\n", p->lineno, p->children[2]->text);
				}
				else {
					return temp;
				}
			}
		}
		//Exp -> ID LP RP
		else if(strcmp(p->children[0]->symbol, "ID") == 0) {
			HashNode* hp = searchHash(p->children[0]->text, Function);
			if(hp == NULL) {
				if(searchHash(p->children[0]->text, Variable) != NULL) {
					printf("Error type 11 at Line %d: \"%s\" is not a function.\n", p->lineno, p->children[0]->text);
				}
				else {
					printf("Error type 2 at Line %d: Undefined function \"%s\".\n", p->lineno, p->children[0]->text);
				}
			}
			else {
				FieldList fieldlist;
				memcpy(&fieldlist, &hp->data, sizeof(fieldlist));
				if(fieldlist->tail != NULL) {
					printf("Error type 9 at Line %d: Function \"%s\" has wrong arguments.\n", p->lineno, p->children[0]->text);
				}
				return fieldlist->type;
			}
		}
		else {
			assert(0);
		}
	}
	else if(p->chnum == 4) {
		//Exp -> Exp LB Exp RB
		if(strcmp(p->children[0]->symbol, "Exp") == 0) {
			Type type_1 = Exp(p->children[0]);
			Type type_2 = Exp(p->children[2]);
			if(type_2 != NULL && type_2->type_kind == BASIC && type_2->basic == INT) {
				if(type_1 != NULL && type_1->type_kind == ARRAY && type_1->array.elem != NULL) {
					return type_1->array.elem;
				}
				else {
					printf("Error type 10 at Line %d: \"%s\" is not an array.\n", p->lineno, p->children[0]->children[0]->text);
				}
			}
			else {
				printf("Error type 12 at Line %d: \"%s\" is not an integer.\n", p->lineno, p->children[2]->children[0]->text);
			}
		}
		//Exp -> ID LP Args RP
		else if(strcmp(p->children[0]->symbol, "ID") == 0) {
			HashNode* hp = searchHash(p->children[0]->text, Function);
		    if(hp == NULL) {
                if(searchHash(p->children[0]->text, Variable) != NULL) {
                    printf("Error type 11 at Line %d: \"%s\" is not a function.\n", p->lineno, p->children[0]->text);
                }
                else {
                    printf("Error type 2 at Line %d: Undefined function \"%s\".\n", p->lineno, p->children[0]->text);
                }
            }
            else {
                FieldList fieldlist;
                memcpy(&fieldlist, &hp->data, sizeof(fieldlist));
				bool args = Args(p->children[2], fieldlist->tail);
                if(!args) {
                    printf("Error type 9 at Line %d: Function \"%s\" has wrong arguments in number or type.\n", p->lineno, p->children[0]->text);
                }
                return fieldlist->type;
            }
		}
	}
	
	return NULL;
	//printf("out exp\n");
}

bool Args(Node* p, FieldList fieldlist) {
	if(fieldlist == NULL) {
		return false;
	}
	Type type = Exp(p->children[0]);
	//Args -> Exp
	if(p->chnum == 1) {
		return typeEqual(fieldlist->type, type) && fieldlist->tail == NULL;
	}
	//Args -> Exp COMMA Args
	else if(p->chnum == 3) {
		bool args = Args(p->children[2], fieldlist->tail);
		return typeEqual(fieldlist->type, type) && args;
	}
	else {
		assert(0);
	}
}



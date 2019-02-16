#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h>
#include <assert.h>
#include <string.h>

#include "IClist.h"
#include "semantic.h"


FILE* fout;

void initOpTable() {
	for(int i=0; i<OP_SIZE; i++) {
		opTable[i] = NULL;
	}
}

Operand insertOp(char* info, int kind) {
	Operand op = (Operand)malloc(sizeof(Operand_));
	op->op_kind = kind;
	op->info = info;
	int i = 0;
	while(opTable[i] != NULL) {
		i++;
	}
	opTable[i] = op;
	return op;
}

Operand searchOp(char* info) {
	for(int i=0; i<OP_SIZE; i++) {
		if(opTable[i] == NULL) {
			return insertOp(info, VARIABLE);
		}
		if(strcmp(opTable[i]->info, info) == 0) {
			return opTable[i];
		}
	}
	return insertOp(info, VARIABLE);
}

ICNode* concatIC(int num, ...) {
	//printf("-----icconcat %d\n", num);
	va_list icList;
	va_start(icList, num);
	ICNode *head1, *head2;
	int i = 1;
	while((head1 = va_arg(icList, ICNode*)) == NULL) {
		i++;
		if(i > num) {
			return NULL;
		}
	}
	for(; i<num; i++) {
		if((head2 = va_arg(icList, ICNode*))) {
			head1->prev->next = head2;
			head2->prev->next = head1;

			ICNode* temp = head1->prev;
			head1->prev = head2->prev;
			head2->prev = temp;
		}
	}
	va_end(icList);
	return head1;
}

ICNode* popIC(ICNode* head) {
	if(head->next == head) {
		return NULL;
	}

	head->prev->prev->next = head;
	head->prev = head->prev->prev;
	return head;
}

char* getOp(Operand op) {
	char* s = (char*)malloc(sizeof(char) * 32);
	if(op->op_kind == CONSTANT) {
		strcat(s, "#");
	}
	strcat(s, op->info);
	return s;
}

void printIC(ICNode* icnode) {
	switch(icnode->ic_kind) {
		case ASSIGN:		fprintf(fout, "%s := %s\n", getOp(icnode->assign.left), getOp(icnode->assign.right));
							break;
		case ASSIGN_ADDR:	fprintf(fout, "%s := &%s\n", getOp(icnode->assign.left), getOp(icnode->assign.right));
							break;
		case ASSIGN_STAR:	fprintf(fout, "%s := *%s\n", getOp(icnode->assign.left), getOp(icnode->assign.right));
							break;
		case STAR_ASSIGN:	fprintf(fout, "*%s := %s\n", getOp(icnode->assign.left), getOp(icnode->assign.right));
							break;
		case ADD:			fprintf(fout, "%s := %s + %s\n", getOp(icnode->binop.result), getOp(icnode->binop.op1), getOp(icnode->binop.op2));
							break;
		case SUB:			fprintf(fout, "%s := %s - %s\n", getOp(icnode->binop.result), getOp(icnode->binop.op1), getOp(icnode->binop.op2));
							break;
		case MUL:			fprintf(fout, "%s := %s * %s\n", getOp(icnode->binop.result), getOp(icnode->binop.op1), getOp(icnode->binop.op2));
							break;
		case DIV:			fprintf(fout, "%s := %s / %s\n", getOp(icnode->binop.result), getOp(icnode->binop.op1), getOp(icnode->binop.op2));
							break;
		case FUNC_DEC:		fprintf(fout, "FUNCTION %s :\n", icnode->name);
							break;
		case PARAM:			fprintf(fout, "PARAM %s\n", getOp(icnode->op));
							break;
		case ARG:			fprintf(fout, "ARG %s\n", getOp(icnode->op));
							break;
		case RET:			fprintf(fout, "RETURN %s\n", icnode->name);
							break;
		case IF:			fprintf(fout, "IF %s %s %s GOTO %s\n", getOp(icnode->cond.left), icnode->cond.op, getOp(icnode->cond.right), icnode->cond.label);
							break;
	    case GOTO:			fprintf(fout, "GOTO %s\n", icnode->name);
							break;
		case LABEL:			fprintf(fout, "LABEL %s :\n", icnode->name);
							break;
		case FUNC_CALL:		fprintf(fout, "%s := CALL %s\n", getOp(icnode->func.place), icnode->func.func_name);
							break;
		case READ:			fprintf(fout, "READ	%s\n", getOp(icnode->op));
							break;
		case WRITE:			fprintf(fout, "WRITE %s\n", getOp(icnode->op));
							break;
		case DEC:			fprintf(fout, "DEC %s %d\n", getOp(icnode->dec.op), icnode->dec.size);
							break;
		default: assert(0);
	}
}

static int icgen_num = 0;
ICNode* newICNode(enum ICKind kind, ...) {
	va_list nodeList;
	va_start(nodeList, kind);

	ICNode *node = (ICNode*)malloc(sizeof(ICNode));
	node->ic_kind = kind;
	//printf("icgen %d %d\n", kind, icgen_num);
	icgen_num++;
	switch(kind) {
		case FUNC_DEC: {
			char* name = va_arg(nodeList, char*);
			node->name = name ;
			break;	
		}
		case ARG: {
			char* name = va_arg(nodeList, char*);
			node->op = searchOp(name);
			break;
		}
		case PARAM: {
			FieldList fl = va_arg(nodeList, FieldList);
			if(fl->type != NULL && (fl->type->type_kind == STRUCTURE || (fl->type->type_kind == ARRAY && fl->type->array.elem != NULL))) {
				node->op = insertOp(fl->name, ADDRESS);
			}
			else {
				node->op = insertOp(fl->name, VARIABLE);
			}	
			break;
		}
		case ASSIGN_STAR: {
			char* left = va_arg(nodeList, char*);
			if(left == NULL) {
				return NULL;
			}
			char* right = va_arg(nodeList, char*);
			node->assign.right = searchOp(right);
			assert(node->assign.right->op_kind == ADDRESS);
			node->assign.left = insertOp(left, VARIABLE);
			break;
		}
		case ASSIGN_ADDR: {
			char* left = va_arg(nodeList, char*);
			if(left == NULL) {
				return NULL;
			}	
			char* right = va_arg(nodeList, char*);
			node->assign.right = searchOp(right);
			assert(node->assign.right->op_kind == VARIABLE);
			node->assign.left = insertOp(left, ADDRESS);
			break;
		}
		case ASSIGN: {
			char* left = va_arg(nodeList, char*);
			if(left == NULL) {
				return NULL;
			}	
			char* right = va_arg(nodeList, char*);
			if(('0' <= right[0] && right[0] <= '9') || right[0] == '+' || right[0] == '-') {
				node->assign.left = insertOp(left, VARIABLE);
				node->assign.right = insertOp(right, CONSTANT);
			}
			else {
				node->assign.right = searchOp(right);
				node->assign.left = insertOp(left, node->assign.right->op_kind);
			}
			break;
		}
		case ADD:
		case SUB:
		case MUL:
		case DIV: {
			char* place = va_arg(nodeList, char*);
			if(place == NULL) {
				return NULL;
			}	
			char* t1 = va_arg(nodeList, char*);
			char* t2 = va_arg(nodeList, char*);
			
			if(('0' <= t1[0] && t1[0] <= '9') || t1[0] == '+' || t1[0] == '-') {
				node->binop.op1 = insertOp(t1, CONSTANT);
			}
			else {
				node->binop.op1 = searchOp(t1);
			}
			
			if(('0' <= t2[0] && t2[0] <= '9') || t2[0] == '+' || t2[0] == '-') {
				node->binop.op2 = insertOp(t2, CONSTANT);
			}
			else {
				node->binop.op2 = searchOp(t2);
			}
			
			if(node->binop.op1->op_kind == ADDRESS || node->binop.op2->op_kind == ADDRESS) {
				node->binop.result = insertOp(place, ADDRESS);
			}
			else {
				node->binop.result = insertOp(place, VARIABLE);
			}
			
			break;
		}
		case NEG: {
			node->ic_kind = SUB;
			char* place = va_arg(nodeList, char*);
			if(place == NULL) {
				return NULL;
			}
			char* t = va_arg(nodeList, char*);
			node->binop.op1 = insertOp("0", CONSTANT);
			node->binop.op2 = searchOp(t);
			node->binop.result = insertOp(place, VARIABLE);
			break;
		}
		case RET: {
			char* t = va_arg(nodeList, char*);
			node->name = t;
			break;
		}
		case GOTO: {
			char* label = va_arg(nodeList, char*);
			node->name = label;
			break;
		}
		case IF: {
			char* t1 = va_arg(nodeList, char*);
			char* t2 = va_arg(nodeList, char*);
			char* op = va_arg(nodeList, char*);
			char* label = va_arg(nodeList, char*);
			node->cond.left = searchOp(t1);
			node->cond.op = op;
			node->cond.right = searchOp(t2);
			node->cond.label = label;
			break;
		}
		case LABEL: {
			char* label = va_arg(nodeList, char*);
			node->name = label;
			break;
		}
		case READ: {
			char* place = va_arg(nodeList, char*);
			node->op = insertOp(place, VARIABLE);
			break;
		}
		case WRITE: {
			char* place = va_arg(nodeList, char*);
			node->op = searchOp(place);
			break;
		}
		case FUNC_CALL: {
			char* place = va_arg(nodeList, char*);
			char* function = va_arg(nodeList, char*);
			node->func.place = insertOp(place, VARIABLE);
			node->func.func_name = function;
			break;
		}
		case DEC: {
			char* place = va_arg(nodeList, char*);
			int size = va_arg(nodeList, int);
			node->dec.op = insertOp(place, VARIABLE);
			node->dec.size = size;
			break;
		}
		default:	assert(0);
	}

	node->prev = node->next = node;
	va_end(nodeList);
	return node;

}




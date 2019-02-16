#ifndef _ICLIST_H_
#define _ICLIST_H_

typedef struct Operand_* Operand;

typedef struct Operand_ {
	enum {VARIABLE, CONSTANT, ADDRESS} op_kind;
	char* info;
}Operand_;

enum ICKind {	
	ASSIGN = 100,
	ASSIGN_ADDR,
	ASSIGN_STAR,
	STAR_ASSIGN,
	ADD,
	SUB,
	MUL,
	DIV,
	FUNC_DEC,
	PARAM,
	ARG,
	RET,
	IF,
	GOTO,
	LABEL,
	FUNC_CALL,
	READ,
	WRITE,
	DEC, 
	NEG
};

typedef struct ICNode {
	enum ICKind ic_kind;
	union {
		struct {Operand left, right;} assign;
		struct {Operand left, right;
				char *op, *label;} cond;
		struct {Operand result, op1, op2;} binop;
		struct {Operand place;
				char* func_name;} func;
		struct {Operand op;
				int size;} dec;
		char* name;
		Operand op;
	};
	struct ICNode *prev, *next;
}ICNode;


#define OP_SIZE 65535

Operand opTable[OP_SIZE];

void initOpTable();
Operand insertOp(char* info, int kind);
Operand searchOp(char* info);
ICNode* concatIC(int num, ...);
ICNode* popIC(ICNode* head);
char* getOp(Operand op);
void printIC(ICNode* icnode);
ICNode* newICNode(enum ICKind kind, ...);

#endif

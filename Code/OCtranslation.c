#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
#include <string.h>
#include <stdbool.h>

#include "IClist.h"
#include "register.h"


extern ICNode* ICroot;
extern FILE* fout;

void OCtransGeneral(ICNode* node) {
	assert(node != NULL);
	switch(node->ic_kind) {
		case ASSIGN: {
			Operand op1 = node->assign.left;
			Operand op2 = node->assign.right;
			if(op2->op_kind == CONSTANT) {
				fprintf(fout, "li %s, %s\n", findReg(op1->info, 1), op2->info);
			}
			else {
				fprintf(fout, "move %s, %s\n", findReg(op1->info, 1), findReg(op2->info, 0));
			}
			break;
		}
		case ASSIGN_ADDR: {
			fprintf(fout, "addi %s, $fp, %d\n", findReg(node->assign.left->info, 1), getVarAddr(node->assign.right->info));
			break;
		}
		case ASSIGN_STAR: {
			fprintf(fout, "lw %s, 0(%s)\n", findReg(node->assign.left->info, 1), findReg(node->assign.right->info, 0));
			break;
		}
		case STAR_ASSIGN: {
			fprintf(fout, "sw %s, 0(%s)\n", findRegs(node->assign.right->info, node->assign.left->info), findReg(node->assign.left->info, 0));
			break;
		}
		case ADD: {
			Operand op1 = node->binop.op1;
			Operand op2 = node->binop.op2;
			Operand rst = node->binop.result;
			if(op2->op_kind == CONSTANT) {
				fprintf(fout, "addi %s, %s, %s\n", findReg(rst->info, 1), findReg(op1->info, 0), op2->info);
			}
			else {
				fprintf(fout, "add %s, %s, %s\n", findReg(rst->info, 1), findRegs(op1->info, op2->info), findReg(op2->info, 0));
			}
			break;
		}
		case SUB: {
			Operand op1 = node->binop.op1;
			Operand op2 = node->binop.op2;
			Operand rst = node->binop.result;
			if(op2->op_kind == CONSTANT) {
				char* t = (char*)malloc(sizeof(char) * (2 + strlen(op2->info)));
				strcpy(t+1, op2->info);
				t[0] = '-';
				fprintf(fout, "addi %s, %s, %s\n", findReg(rst->info, 1), findReg(op1->info, 0), t);
			}
			else {
				fprintf(fout, "sub %s, %s, %s\n", findReg(rst->info, 1), findRegs(op1->info, op2->info), findReg(op2->info, 0));
			}
			break;
		}
		case MUL:
		case DIV: {
			Operand op1 = node->binop.op1;
			Operand op2 = node->binop.op2;
			Operand rst = node->binop.result;
			char* v = op2->info;
			if(op2->op_kind == CONSTANT) {
				fprintf(fout, "li %s, %s\n", findReg("@temp", 1), op2->info);
				v = "@temp";
			}
			if(node->ic_kind == MUL) {
				fprintf(fout, "mul %s, %s, %s\n", findReg(rst->info, 1), findRegs(op1->info, v), findReg(v, 0));
			}
			else {
				fprintf(fout, "div %s, %s\n", findRegs(op1->info, v), findReg(v, 0));
				fprintf(fout, "mflo %s\n", findReg(rst->info, 1));
			}
			break;
		}
		//case PARAM:
		//case ARG:
		case RET: {
			fprintf(fout,"move $sp, $fp\n");

			fprintf(fout,"lw $fp, 0($sp)\n");
			fprintf(fout,"addi $sp, $sp, 4\n");

			fprintf(fout,"lw $ra, 0($sp)\n");
			fprintf(fout,"addi $sp, $sp ,4\n");
		
			fprintf(fout,"move $v0, %s\n", findReg(node->name, 0));
			fprintf(fout,"jr $ra\n");

			break;
		}
		case IF: {
			storeDirty();
			char* op = node->cond.op;
			char* s1 = findReg(node->cond.right->info, 0);
			char* s2 = findRegs(node->cond.left->info, node->cond.right->info);
			
			if(strcmp(op, "==") == 0)
				fprintf(fout, "beq");
			else if(strcmp(op, "!=") == 0)
				fprintf(fout, "bne");
			else if(strcmp(op, ">") == 0)
				fprintf(fout, "bgt");
			else if(strcmp(op, "<") == 0)
				fprintf(fout, "blt");
			else if(strcmp(op, ">=") == 0)
				fprintf(fout, "bge");
			else if(strcmp(op, "<=") == 0)
				fprintf(fout, "ble");
			fprintf(fout, " %s, %s, %s\n", s2, s1, node->cond.label);
			break;
		}
		case GOTO: {
			storeDirty();
			fprintf(fout, "j %s\n", node->name);
			break;
		}
		case LABEL: {
			storeDirty();
			initRegister();
			fprintf(fout, "%s:\n", node->name);
			break;
		}
		//case FUNC_CALL:
		case READ: {
			fprintf(fout, "jal read\n");
			fprintf(fout, "move %s, $v0\n", findReg(node->op->info, 1));
			break;
		}
		case WRITE: {
			fprintf(fout, "move $a0, %s\n", findReg(node->op->info, 0));
			fprintf(fout, "jal write\n");
			break;
		}
		case DEC: {
			st_top -= node->dec.size;
			newVarAddr(node->dec.op->info, st_top);
			break;
		}
		case NEG: {
			printf("object code fault - NEG\n");
			assert(0);
			break;
		}
		default:fprintf(fout,"\n");
	}
}

ICNode* OCtransArg_Call(ICNode* p) {
	storeDirty();
	
	//ARG
	ICNode* t = p;
	int cnt = 0;
	while(t->ic_kind == ARG){
		t = t->next;
		cnt++;
	}
	int temp = cnt;
	for(; cnt>=5; cnt--){
		fprintf(fout, "addi $sp, $sp, -4\n");
		fprintf(fout, "lw %s,0($sp)\n", findReg(p->op->info, 0));
		p = p->next;
	}
	for(; cnt>=1; cnt--){
		fprintf(fout, "move $a%d, %s\n", cnt-1, findReg(p->op->info, 0));
		p = p->next;
	}
	assert(p == t);
	
	//CALL
	initRegister();
	fprintf(fout, "jal %s\n", p->func.func_name);
	for(; temp>=5; temp--) {
		fprintf(fout, "addi $sp, $sp, 4\n");
	}
	fprintf(fout, "move %s, $v0\n", findReg(p->func.place->info, 1));
	p = p->next;

	return p;
}

int varNum(ICNode* p) {
	int cnt = 0;
	ICNode* t = p->next;
	while(t->ic_kind != FUNC_DEC){
		if(t->ic_kind == DEC) {
			cnt += t->dec.size/4;
		}
		else if(t->ic_kind != ARG &&
				t->ic_kind != RET &&
				t->ic_kind != LABEL &&
				t->ic_kind != IF &&
				t->ic_kind != GOTO &&
				t->ic_kind != STAR_ASSIGN &&
				t->ic_kind != WRITE) {
			cnt++;
		}
		t = t->next;
	}
	return cnt;	
}

ICNode* OCtransFunc_Param(ICNode* p) {
	//FUNC_DEC
	int cnt = varNum(p);

	initRegister();
	
	fprintf(fout, "\n%s:\n", p->name);
	
	fprintf(fout, "addi $sp, $sp, -4\n");
	fprintf(fout, "sw $ra, 0($sp)\n");
	fprintf(fout, "addi $sp, $sp, -4\n");
	fprintf(fout, "sw $fp, 0($sp)\n");
	fprintf(fout, "move $fp, $sp\n");	
	st_top = 0;

	fprintf(fout, "addi $sp, $sp, %d\n", -cnt*4);
	p = p->next;

	//PARAM
	if(p->ic_kind == PARAM) {
		int cnt = 0;
		while(p->ic_kind == PARAM) {
			if(cnt <= 3) {
				fprintf(fout, "move %s,$a%d\n", findReg(p->op->info, 1), cnt);
			}
			else {
				fprintf(fout, "lw %s, %d($sp)\n", findReg(p->op->info, 1), (cnt-4+2)*4);
			}
			cnt ++;
			p = p->next;
		}
	}

	return p;
}


void printRead_Write() {
	fprintf(fout,"\
.data\n\
_prompt: .asciiz \"Enter an integer:\"\n\
_ret: .asciiz \"\\n\"\n\
.globl main\n\
.text\n\
read:\n\
li $v0, 4\n\
la $a0, _prompt\n\
syscall\n\
li $v0, 5\n\
syscall\n\
jr $ra\n\
\n\
write:\n\
li $v0, 1\n\
syscall\n\
li $v0, 4\n\
la $a0, _ret\n\
syscall\n\
move $v0, $0\n\
jr $ra\n\
");
}

void printOC() {
	printRead_Write();
	ICNode* p = ICroot;
	while(1) {
		if(p->ic_kind == ARG || p->ic_kind == FUNC_CALL) {
			p = OCtransArg_Call(p);
		}
		else if(p->ic_kind == FUNC_DEC) {
			p = OCtransFunc_Param(p);
		}
		OCtransGeneral(p);
		p = p->next;
		if(p == ICroot)
			break;
	}
}

#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <time.h>
#include "register.h"

#define REG_NUM 18
char* regs[REG_NUM];
int dirty[REG_NUM];

extern FILE* fout;
VarDesc *vars = NULL;

int st_top = 0;

void initRegister() {
	for(int i=0; i<REG_NUM; i++) {
		regs[i] = NULL;
		dirty[i] = 0;
	}
}

void newVarAddr(char* name, int offset) {
	if(vars == NULL) {
		vars = malloc(sizeof(VarDesc));
		vars->name = name;
		vars->offset = offset;
		vars->next = NULL;
	}
	else {
		VarDesc* p = vars;
		while(p->next != NULL) {
			p = p->next;
		}
		p->next = malloc(sizeof(VarDesc));
		p->next->name = name;
		p->next->offset = offset;
		p->next->next = NULL;
	}
}

int getVarAddr(char* name) {
	VarDesc* p = vars;
	while(p != NULL) {
		if(strcmp(p->name, name) == 0) {
			return p->offset;
		}
		p = p->next;
	}
	st_top = st_top - 4;
	newVarAddr(name, st_top);
	return st_top;
}

char* regName(int n) {
	char* s = malloc(sizeof(char) * 4);
	if(n < 10) {
		sprintf(s, "$t%d", n);
	}
	else {
		sprintf(s, "$s%d", n-10);
	}
	return s;
}

int allocateReg(char* name) {
	for(int i=0; i<REG_NUM; i++) {
		if(regs[i] == NULL) {
			regs[i] = name;
			return i;
		}
	}
	srand(time(0));
	int r = (int)rand()%18;
	int offset = getVarAddr(regs[r]);
	fprintf(fout, "sw %s, %d($fp)\n", regName(r), offset);
	regs[r] = name;
	return r;
}

int allocateRegAvoid(char* name, int avoid) {
	for(int i=0; i<REG_NUM; i++) {
		if(regs[i] == NULL) {
			regs[i] = name;
			return i;
		}
	}
	int r;
	srand(time(0));
	while(1) {
		r = (int)rand()%18;
		if(r != avoid) {
			break;
		}
	}
	int offset = getVarAddr(regs[r]);
	fprintf(fout, "sw %s, %d($fp)\n", regName(r), offset);
	regs[r] = name;
	return r;
}

char* findReg(char* name, int store) {
	if(strcmp(name, "0") == 0) {
		return "$zero";
	}
	int r = -1;
	for(int i=0; i<REG_NUM; i++) {
		if(regs[i] != NULL) {
			if(strcmp(name, regs[i]) == 0) {
				r = i;
				if(store) {
					dirty[r] = 1;
				}
				break;
			}
			}
	}
	if(r == -1) {
		r = allocateReg(name);
		if(store) {
			dirty[r] = 1;
		}
		else {
			int offset = getVarAddr(name);
			fprintf(fout, "lw %s, %d($fp)\n", regName(r), offset);
		}
	}
	return regName(r);
}

char* findRegs(char* name1, char* name2) {
	if(strcmp(name1, "0") == 0) {
		return "$zero";
	}
	int r = -1;
	int avoid = -1;
	for(int i=0; i<REG_NUM; i++) {
		if(regs[i] != NULL) {
			if(strcmp(name1, regs[i]) == 0) {
				r = i;
			}
			if(strcmp(name2, regs[i]) == 0) {
				avoid = i;
			}
		}
	}

	if(r == -1) {
		r = allocateRegAvoid(name1, avoid);
		int offset = getVarAddr(name1);
		fprintf(fout, "lw %s, %d($fp)\n", regName(r), offset);
	}
	return regName(r);
}

void storeDirty() {
	for(int i=0; i<REG_NUM; i++) {
		if(dirty[i]) {
			int offset = getVarAddr(regs[i]);
			fprintf(fout,"sw %s, %d($fp)\n", regName(i), offset);
			dirty[i] = 0;
		}
	}
}

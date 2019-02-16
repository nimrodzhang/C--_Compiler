#ifndef _REGISTER_H_
#define _REGISTER_H_

typedef struct VarDesc {
	char* name;
	int offset;
	struct VarDesc* next;
}VarDesc;

extern int st_top;
void initRegister();
void newVarAddr(char*, int);
int getVarAddr(char*);
char* findReg(char*, int);
char* findRegs(char*, char*);
void storeDirty();

#endif

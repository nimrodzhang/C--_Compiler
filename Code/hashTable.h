#ifndef _HASHTABLE_H_
#define _HASHTABLE_H_

#include "semantic.h"

#define HASH_SIZE 65535

typedef struct HashNode {
	char* name;
	//enum for search in hash table: hash_kind
	enum {Variable, Structure, Function} hash_kind;
	union {
		Type type;				//for Variable
		FieldList fieldlist;	//for Structure or Function
	}data;
	int lineno;					//for Function
}HashNode;


void initHash();
unsigned int hashPJW(char* name);
int insertHash(char* name, int s_kind, void* data, int lineno);
HashNode* searchHash(char* name, int s_kind);
void checkHash();
void hashPrint();

HashNode* hashTable[HASH_SIZE];

#endif

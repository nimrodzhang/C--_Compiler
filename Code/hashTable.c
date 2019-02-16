#include "hashTable.h"

static void addConstantFunc() {
	FieldList fl1 = (FieldList)malloc(sizeof(FieldList_));
	fl1->name = "return";
	fl1->type = (Type)malloc(sizeof(Type_));
	fl1->type->type_kind = BASIC;
	fl1->type->basic = INT;
	fl1->tail = NULL;
	insertHash("read", Function, fl1, 0);	

	FieldList fl2 = (FieldList)malloc(sizeof(FieldList_));
	fl2->name = "return";
	fl2->type = fl1->type;
	fl2->tail = (FieldList)malloc(sizeof(FieldList_));
	fl2->tail->name = NULL;
	fl2->tail->type = fl1->type;
	fl2->tail->tail	= NULL;
	insertHash("write", Function, fl2, 0);
}

void initHash() {
	for(int i=0; i<HASH_SIZE; i++) {
		hashTable[i] = NULL;
	}
	addConstantFunc();
}

unsigned int hashPJW(char* name) {
	unsigned int value, temp;
	for(value=0; *name; ++name) {
		value = (value << 2) + *name;
		if((temp = (value & ~0x3fff))) {
			value = (value ^ (temp >> 12)) & 0x3fff;
		}
	}
	return value % HASH_SIZE;
}

int insertHash(char* name, int hash_kind, void* data, int lineno) {
	HashNode *p = (HashNode*)malloc(sizeof(HashNode));
	
	//fill data
	p->name = name;
	p->hash_kind = hash_kind;
	memcpy(&p->data, &data, sizeof(data));
	p->lineno = lineno;

	//put into hash table
	unsigned int key = hashPJW(name);
	int collisions = 0;

	while(collisions < HASH_SIZE) {
		if(hashTable[key] == NULL) {
			hashTable[key] = p;
			return 1;
		}
		//else 
		key++;
		key = key % HASH_SIZE;
		collisions++;
	}

	//fail because of table full
	printf("Hash Table Full!\n");
	return 0;
}

HashNode* searchHash(char* name, int hash_kind) {
	unsigned int key = hashPJW(name);
	HashNode* p = hashTable[key];

	while(p != NULL) {
		if(strcmp(p->name, name) == 0 && p->hash_kind == hash_kind) {
			return p;
		}
		//else
		key++;
		key = key % HASH_SIZE;
		p = hashTable[key];
	}
	//search fail
	return NULL;
}

void checkHash() {
	HashNode* p = NULL;
	for(int i=0; i<HASH_SIZE; i++) {
		p = hashTable[i];
		if(p != NULL) {
			if(p->hash_kind == Function && p->lineno != 0) {
				printf("Error type 18 at line %d: Undefined function \"%s\".\n", p->lineno, p->name);
			}
		}
	}
}


//for test
void hashPrint() {
	for(int i=0; i<HASH_SIZE; i++) {
		if(hashTable[i] != NULL) {
			printf("name:%s, kind:%d\n", hashTable[i]->name, hashTable[i]->hash_kind);
		}
	}
}

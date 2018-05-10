#ifndef SYMTABLE_H
#define SYMTABLE_H

#include <stdbool.h>
#include "type.h"

struct tree;
struct typeinfo;
struct list;

struct symbol {
	char *name;
	struct typeinfo *type;
	struct symTable *table;
	bool funcDecl; 
};

struct symTable {
	enum region scope;
	int tSize;
	struct symTable *parent;
	struct symbol **symbols;
};

struct tableList {
	struct symTable *table;
	struct tableList *next;
};

struct symTable *current;
struct symTable *global;
struct symTable *constant;
struct tableList *tableHead;

unsigned long hash(char *str);
int key (char *str, int tSize);
bool newSymbol(char *name, struct typeinfo *type, struct symTable *table, bool funcDef, bool lit, struct tree *t);
struct symTable *newTable(enum region scope, int tSize);
bool checkDecl(char *name, struct symTable *table);
void populateSymTable(struct tree *t);
void populateLocals(struct tree *t);
void populateDeclarators(struct tree *t, struct typeinfo *type);
char *getFunction(struct tree *t);
char *getClass(struct tree *t);
struct tree *getProduction(struct tree *t, int rule);
struct tree *productionBefore(struct tree *t, int ruleFind, int ruleBefore);
void populateParameters(struct tree *t, struct typeinfo *type, struct list **parameters, bool funcDecl);
void populateClass(struct tree *t, struct typeinfo *type);
struct symbol *getSymbol(char *name, struct symTable *table);
void populateList(struct tree *t, struct list **list);
int paramNum(struct list *list);
int tableSize(struct symTable *table);
void printTable(struct symTable *table);


#endif

#ifndef INTERMEDIATE_H
#define INTERMEDIATE_H

#include <stdio.h>
#include "type.h"

struct tree;
struct typeinfo;

enum opcode {
	O_ADD,
	O_SUB,
	O_MUL,
	O_DIV,
	O_NEG,
	O_ASN,
	O_ADDR,
	O_LCONT,
	O_SCONT,
	O_GOTO,
	O_BLT,
	O_BLE,
	O_BGT,
	O_BGE,
	O_BEQ,
	O_BNE,
	O_BIF,
	O_PARM,
	O_CALL,
	O_RET,
	O_GLOB,
	O_PROC,
	O_LOCAL,
	O_LABEL,
	O_END,
	O_ERR,
	O_AND,
	O_OR,
	O_CALLC,
	O_DEREF
};

struct op {
	char *name;
	enum opcode opcode;
	struct address address[3];
};

void gen(struct tree *t);
struct op *alctac(enum opcode code, struct address a, struct address b, struct address c, char *name);
void addtac(struct tree *t, struct op *op);
void printaddress(FILE *file, struct address address);
char *regionstring(enum region region);
char *opcodestring(enum opcode code);
void printCodeList(FILE *file, struct list *codeList);
struct address getAddress(struct tree *t, int child);
enum opcode getOpcode(int label);
struct address newtemp(struct typeinfo *type);
void statements(struct list *code, struct op *first, struct op *follow);
struct op *newlabel();


#endif

	

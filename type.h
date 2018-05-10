#ifndef TYPE_H
#define TYPE_H

#include <stdbool.h>

struct tree;
struct param;

enum type {
	INT_T,
	FLOAT_T,
	CHAR_T,
	STRING_T,
	BOOL_T,
	ARRAY_T,
	FUNCTION_T,
	CLASS_T,
	VOID_T, 
	UNKNOWN_T
};

enum region {
	GLOBAL_R,
	LOCAL_R,
	CLASS_R,
	UNKNOWN_R,
	CONST_R,
	PARAM_R,
	LABEL_R
};

struct address {
	enum region region;
	int offset;
	struct typeinfo *type;
};

struct typeinfo {
	enum type basetype;
	bool pointer;
	struct address place;
	struct token *token;
	
	union {
		struct arrayinfo {
			struct typeinfo *type;
			int size;
		} array;
		struct functioninfo {
			struct typeinfo *returntype;
			struct list *parameters;
			int paramSize;
		} function;
		struct classinfo {
			char *type;
		} class;
	};
};



void setBaseTypes();
struct typeinfo *alctype(struct tree *t);
enum type returnType(int type); 
struct typeinfo *alctypeArray(struct tree *t, struct typeinfo *type);
struct typeinfo *alctypeFunction(struct tree *t, struct typeinfo *type);
int getTypeSize(struct typeinfo *type);
void printType(enum type type);
bool typeinfoCompare(struct typeinfo *l, struct typeinfo *r);
struct typeinfo *typeinfoCopy(struct typeinfo *type);
bool compareLists(struct list *l, struct list *r);
struct typeinfo *getRight(struct tree *t);
struct typeinfo *getLeft(struct tree *t);
struct typeinfo *getMiddle(struct tree *t);
struct typeinfo *typecheck(struct tree *t);

#endif

#include "symtable.h"
#include "tree.h"
#include "120gram.h"
#include <stdbool.h>
#include "120lex.h"
#include "list.h"

extern struct typeinfo int_t;
extern struct typeinfo float_t;
extern struct typeinfo char_t;
extern struct typeinfo array_t;
extern struct typeinfo bool_t;
extern struct typeinfo class_t;
extern struct typeinfo void_t;
extern struct typeinfo unknown_t;
extern struct typeinfo string_t;
extern int offset;


/* sdbm hashing algorithm found here http://www.cse.yorku.ca/~oz/hash.html*/
unsigned long sdbm(char *str){
        unsigned long hash = 0;
        int c;

        while ((c = *str++))
            hash = c + (hash << 6) + (hash << 16) - hash;

        return hash;
}

/* Function that returns key for entry in symbol table */
int getKey(char *str, int tSize){	
	int key;
	key = sdbm(str) % tSize;

	return key;
}

/* 
 * Allocates new symbol and enters it into the given symTable
 * based on the given key. 
 */
bool newSymbol(char *name, struct typeinfo *type, struct symTable *table, bool funcDecl, bool lit, struct tree *t){	
	int key = getKey(name, table->tSize);	
	struct symbol *s = malloc(sizeof(struct symbol));
	s->name = name;
	s->table = NULL;
	s->funcDecl = funcDecl;
	s->type = type;
	s->type->place.region = t->place.region = table->scope;
	s->type->place.offset = t->place.offset = offset;
	s->type->place.type = t->place.type = type;
	table->symbols[key] = s;
	if(lit)
		s->type->place.offset = t->place.offset = type->token->ival;
	else
		offset += getTypeSize(type);
	//printf("OFFSET: %d\n", t->place.offset);
	//printf("SYMBOL ADD: %s\n", name);

	
	return false;
}

/* Checks for function declarations */
bool checkDecl(char *name, struct symTable *table){
	int key = getKey(name, table->tSize);
	if(table->symbols[key] && table->symbols[key]->funcDecl == true)
		return false;
	else if(table->symbols[key])
		return true;
	else
		return false;
}

/* Looks for symbol current and parent table */
struct symbol *getSymbol(char *name, struct symTable *table) {
	int key = getKey(name, table->tSize);	
	if(table->symbols[key])
		return table->symbols[key];
	if(table->parent != NULL) {
		int key = getKey(name, table->parent->tSize);
		if(table->parent->symbols[key])
			return table->parent->symbols[key];	
	}
	return NULL;	
}

/* 
 * Given a rule, find the first instance of that rule. If not found
 * returns null.
 */
struct tree *getProduction(struct tree *t, int rule) {
	int i;
	if (t == NULL)
		return NULL;
	if (t->nkids != -1 && t->label == rule)		
		return t;
	for(i = 0; i < t->nkids; i++) {
		struct tree *temp = getProduction(t->kids[i], rule);
		if(temp)
			return temp;
	}
	return NULL;
	
}

/* Given a rule, looks for rule before another give rule */
struct tree *productionBefore(struct tree *t, int ruleFind, int ruleBefore){
	int i;
	if (t == NULL)
		return NULL;
	if (t->nkids != -1 && t->label == ruleFind)		
		return t;
	if (t->nkids != -1 && t->label == ruleBefore)
		return NULL;	
	for(i = 0; i < t->nkids; i++) {
		struct tree *temp = productionBefore(t->kids[i], ruleFind, ruleBefore);
		if(temp)
			return temp;
	}
	return NULL;
}

/* Allocates new table, entires for table, and sets the scope */
struct symTable *newTable(enum region scope, int tSize){
	struct symTable *t = malloc(sizeof(struct symTable));
	t->symbols = calloc(tSize, sizeof(struct symbol));
	t->scope = scope;
	t->tSize = tSize;	
	t->parent = NULL;
	return t;
}

/* Returns size of given table */
int tableSize(struct symTable *table) {
	int total = 0;
	int i;

	if(table == NULL)
		return 0;
	for(i = 0; i < table->tSize; i++) {
		if (table->symbols[i] != NULL){
			total += getTypeSize(table->symbols[i]->type);}
	}
	return total;	
}

void printTable(struct symTable *table) {
	int i;
	for(i = 0; i < table->tSize; i++) {
		if (table->symbols[i] != NULL){
			printf("SYMBOL: %s\n", table->symbols[i]->name);
		}
	}
}
/* Handles local variables in functions and classes */
void populateLocals(struct tree *t){
	int i;
	if (t == NULL)
		return;

	switch(t->label){
		case simple_declaration_1: 
			populateSymTable(t);
			return;
		case primary_expression_4:
			/* Added case to prevent variable being accessed before being declared
			  This isn't checked during type checking */
			if(!getSymbol(t->kids[0]->leaf->text, current))
				semantic_error("variable undeclared", t);
			return;
		case primary_expression_1:
			if (!getSymbol(t->kids[0]->leaf->text, constant))
				newSymbol(t->kids[0]->leaf->text, alctype(t), constant, false, true, t);
			return;
		default:
      			for (i=0; i < t->nkids; i++)
	 			populateLocals(t->kids[i]);		
	}		
	
}

/* Handles methods and variables in classes */
void populateClass(struct tree *t, struct typeinfo *type) {
	int i;

	if (t == NULL)
		return;

	switch(t->label) {
		case member_declaration_1:
			populateClass(t->kids[1], alctype(t));
			return;
		/* Arrays */
		case direct_declarator_8:
			populateClass(t->kids[0], alctypeArray(t, type));
			return;
		/* Function prototypes */
		case direct_declarator_4: {
			struct tree *name = getProduction(t, direct_declarator_1);
			struct typeinfo *decl = alctypeFunction(t, type);
			struct list *declparam = NULL;
			populateParameters(t, type, &declparam, true);
			decl->function.parameters = declparam;
			newSymbol(name->kids[0]->leaf->text, decl, current, true, false, t);
			return;
		}
		/* Constructor prototypes */
		case direct_declarator_5: { 
			struct typeinfo *retType = alctype(t);
			retType->basetype = CLASS_T;
			retType->class.type = t->kids[0]->leaf->text;
			struct typeinfo *ctor = alctypeFunction(t, retType);
			struct list *ctorparam = NULL;
			populateParameters(t, type, &ctorparam, true);
			ctor->function.parameters = ctorparam;
			newSymbol(t->kids[0]->leaf->text, ctor, current, true, false, t);
			return;
		}
		/* identifier */
		case direct_declarator_1:
			if (type == NULL)
				semantic_error("does not name a type", t);			
			if (getSymbol(t->kids[0]->leaf->text, current))
				semantic_error("variable redeclared", t);
			else
				newSymbol(t->kids[0]->leaf->text, type, current, false, false, t);
			return;
		case function_definition_1:
			populateSymTable(t);
			return;
		case ctor_function_definition: {
			int key;
			char *func;
			func = getFunction(t->kids[0]);
			if (!checkDecl(func, current)) {
				struct symbol *funcDecl = getSymbol(func, current);
				int offsettemp = offset;
				offset = 0;
				struct typeinfo *retType;
				retType = alctype(t);
				retType->basetype = CLASS_T;
				retType->class.type = func;
				struct typeinfo *funcType;
				funcType = alctypeFunction(t, retType);
				newSymbol(func, funcType, current, false, false, t);
				struct symbol *funcSym = getSymbol(func, current);
				key = getKey(func, current->tSize);
				current->symbols[key]->table = newTable(LOCAL_R, 1009);
				current->symbols[key]->table->parent = current;
				current = current->symbols[key]->table;
				struct list *parameters = NULL;
				populateParameters(t->kids[0], NULL, &parameters, false);
				if(funcDecl != NULL) {
					if(paramNum(parameters) != paramNum(funcDecl->type->function.parameters))
						semantic_error("number of function declaration parameters differs from definiton", t);
					if(!compareLists(parameters, funcDecl->type->function.parameters))
						semantic_error("parameter types from function declaration do not match", t);
					
				}		
				funcSym->type->function.parameters = parameters;
				funcSym->type->function.paramSize = offset;
				offset = 0;
				populateLocals(t->kids[2]);			
				current = current->parent;
				offset = offsettemp;
				return;
		
			} else {
				semantic_error("function redeclared", t);
				return;
			}			
		}	
		default:
      			for (i=0; i < t->nkids; i++)
	 			populateClass(t->kids[i], type);
      					
				
	}
}

/* A majority of the actual symbols being added to symbol table happens here */
void populateDeclarators(struct tree *t, struct typeinfo *type) {
	int i;	
	if (t == NULL)
		return;	
	switch(t->label) {
		case class_specifier:
			populateSymTable(t);
			return;	
		/* Arrays */
		case direct_declarator_8:
			populateDeclarators(t->kids[0], alctypeArray(t, type));
			return;
		/* Function prototypes */
		case direct_declarator_4: {
			struct tree *name = getProduction(t, direct_declarator_1);
			struct typeinfo *decl = alctypeFunction(t, type);
			struct list *declparam = NULL;
			populateParameters(t, type, &declparam, true);
			decl->function.parameters = declparam;
			newSymbol(name->kids[0]->leaf->text, decl, current, true, false, t);
			return;
		}
		case declarator:
			populateDeclarators(t->kids[1], type);
			return;
		/* identifier */
		case direct_declarator_1:
			if (getSymbol(t->kids[0]->leaf->text, current))
				semantic_error("variable redeclared", t);
			else
				newSymbol(t->kids[0]->leaf->text, type, current, false, false, t);
			return;
		case primary_expression_1:
			if (!getSymbol(t->kids[0]->leaf->text, constant))
				newSymbol(t->kids[0]->leaf->text, alctype(t), constant, false, true, t);
			return;
		default:
      			for (i=0; i < t->nkids; i++)
	 			populateDeclarators(t->kids[i], type);
	}	
	
}

/* Grabs function name */
char *getFunction(struct tree *t) {
	while(1){
		switch(t->label) {
			case direct_declarator_4:
				t = t->kids[0];
				break;
			case direct_declarator_5:
				return t->kids[0]->leaf->text;
			case direct_declarator_6:
				return t->kids[2]->leaf->text;
			case direct_declarator_7:
				return t->kids[2]->leaf->text;
			case direct_declarator_1:
				return t->kids[0]->leaf->text;
			default:
				return NULL;
		}
	}
}

/* Grabs class name */
char *getClass(struct tree *t) {	
	switch(t->label) {
			case class_head:
				return t->kids[1]->leaf->text;
			default:
				return NULL;
		} 
}

/* Grabs parameters from function */
void populateParameters(struct tree *t, struct typeinfo *type, struct list **parameters, bool funcDecl){	
	int i;

	if (t == NULL)
		return;
	switch(t->label) {	
		case direct_declarator_4:
			populateParameters(t->kids[2], type, parameters, funcDecl);
			return;
		case direct_declarator_5:
			populateParameters(t->kids[2], type, parameters, funcDecl);
			return;
		case parameter_declaration_list:
			populateParameters(t->kids[0], type, parameters, funcDecl);
			populateParameters(t->kids[2], type, parameters, funcDecl);
			return;
		case parameter_declaration:
			/* If identifer not given with type, just add type to list */
			if(getProduction(t, direct_declarator_1))
				populateParameters(t->kids[1], alctype(t), parameters, funcDecl);
			else
				alclistback(alctype(t), parameters);
			return;
		/* array */
		case direct_declarator_8:
			populateParameters(t->kids[0], alctypeArray(t, type), parameters, funcDecl);
			return;
		/* identifier */
		case direct_declarator_1:
			/* 
			 * If populating for function declaration, just add types and don't add identifiers
			 * to symbol table
			 */
			if(funcDecl)
				alclistback(type, parameters);
			else {	
				if (getSymbol(t->kids[0]->leaf->text, current))
					semantic_error("variable redeclared", t);
				else {
					newSymbol(t->kids[0]->leaf->text, type, current, false, false, t);
					alclistback(type, parameters);	
				}
			}
			return;	
		default:
			for (i=0; i < t->nkids; i++)
	 			populateParameters(t->kids[i], type, parameters, funcDecl);
	}
}

/* 
 * Big function to populate all symbol tables needed. 
 * Handles classes, global variables, and functions. 
 * Many special cases for functions, variables, 
 * and classes not handled as they aren't included in 120++
 */ 
void populateSymTable(struct tree *t){
	int key;
	int i;
	char *func;
	char *class;
	
	setBaseTypes();
	if (t == NULL)
		return;
	
	switch(t->label) {
		case function_definition_1: {
			struct symTable *current_ = current;
			struct tree *classnode;
			func = getFunction(t->kids[1]);
			/* Make sure to switch to class table if function is from class */
			if((classnode = getProduction(t, direct_declarator_6)) != NULL) {
				struct symbol *class = getSymbol(classnode->kids[0]->leaf->text, current);
				current = class->table;
			}
			if (!checkDecl(func, current)) {
				struct list *parameters = NULL;
				struct symbol *funcDecl = getSymbol(func, current);
				int offsettemp = offset;
				offset = 0;
				struct typeinfo *retType;
				retType = alctype(t);
				struct typeinfo *funcType;
				funcType = alctypeFunction(t, retType);
				newSymbol(func, funcType, current, false, false, t);
				struct symbol *funcSym = getSymbol(func, current);
				key = getKey(func, current->tSize);
				current->symbols[key]->table = newTable(LOCAL_R, 1009);
				current->symbols[key]->table->parent = current;
				current = current->symbols[key]->table;
				populateParameters(t->kids[1], NULL, &parameters, false);
				if(funcDecl != NULL) {
					if(paramNum(parameters) != paramNum(funcDecl->type->function.parameters))
						semantic_error("number of function declaration parameters differs from definiton",
								 getProduction(t, direct_declarator_1));
					if(!compareLists(parameters, funcDecl->type->function.parameters))
						semantic_error("parameter types from function declaration do not match", 
								getProduction(t, direct_declarator_1));
				}	
				funcSym->type->function.parameters = parameters;
				funcSym->type->function.paramSize = offset;
				populateLocals(t->kids[3]);			
				current = current_;
				offset = offsettemp;
				return;
			
			} else {
				semantic_error("function redeclared", getProduction(t, direct_declarator_1));
				return;
			}
		}			
		case ctor_function_definition: {	
			class = getFunction(t->kids[0]);
			struct symbol *classSym = getSymbol(class, current);
			current = classSym->table;
			populateClass(t, NULL);
			current = current->parent;
			return;
		}
		case class_specifier: {
			class = getClass(t->kids[0]);
			if (getSymbol(class, current) == NULL) {
				int offsettemp = offset;
				offset = 0;
				struct typeinfo *classType;
				classType = alctype(t->kids[0]);	
				classType->class.type = class;
				newSymbol(class, classType, current, false, false, t);
				key = getKey(class, current->tSize);
				current->symbols[key]->table = newTable(CLASS_R, 1009);
				current->symbols[key]->table->parent = current;
				current = current->symbols[key]->table;	
				populateClass(t->kids[2], NULL);	
				current = current->parent;
				offset = offsettemp;
				return;	
			} else {
				/* If function is redeclared it will output a syntax error instead */
				semantic_error("class redeclared", t);
				return;
			}
		}
		case simple_declaration_1:
			if (t->kids[0]->label == class_specifier)
				populateSymTable(t->kids[0]);
			else
				populateDeclarators(t->kids[1], alctype(t));
			return;
		default:
      			for (i=0; i < t->nkids; i++)
	 			populateSymTable(t->kids[i]);
     	 }

	
}



/* Populates list for things such as parameters and init_lists */
void populateList(struct tree *t, struct list **list) {	
	int i;	
	struct symbol *entry;		

	if(t == NULL)
		return;	
	switch(t->label) {
		case postfix_expression_2:
			t = t->kids[0];			
				if ((entry = getSymbol(t->kids[0]->leaf->text, current)) != NULL)
					alclistback(entry->type->function.returntype, list);
			return;
		case primary_expression_1:
			alclistback(alctype(t), list);
			return;
		case primary_expression_4:		
				if ((entry = getSymbol(t->kids[0]->leaf->text, current)) != NULL)
					alclistback(entry->type, list);
				else
					alclistback(&unknown_t, list);	
			return;
		default:
			if(t->label > 0)
				;
			else { 
				for (i=0; i < t->nkids; i++)
	 				populateList(t->kids[i], list);
			}
			return;
	}
}









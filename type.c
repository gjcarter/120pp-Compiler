#include "symtable.h"
#include "tree.h"
#include "120gram.h"
#include "type.h"
#include "list.h"

struct typeinfo int_t;
struct typeinfo float_t;
struct typeinfo char_t;
struct typeinfo array_t;
struct typeinfo bool_t;
struct typeinfo class_t;
struct typeinfo void_t;
struct typeinfo unknown_t;
struct typeinfo string_t;
struct typeinfo ptr_t;
extern bool saw_fstream;
extern bool saw_iostream;
extern bool saw_std;

/* Sets some base types to be used during typechecking */
void setBaseTypes() {
	int_t.basetype = INT_T;
	int_t.pointer = false;
	
	float_t.basetype = FLOAT_T;
	float_t.pointer = false;
	
	char_t.basetype = CHAR_T;
	char_t.pointer = false;

	bool_t.basetype = BOOL_T;
	bool_t.pointer = false;

	class_t.basetype = CLASS_T;
	class_t.pointer = false;

	void_t.basetype = VOID_T;
	void_t.pointer = false;

	unknown_t.basetype = UNKNOWN_T;
	unknown_t.pointer = false;

	string_t.basetype = CHAR_T;
	string_t.pointer = true;

	ptr_t.basetype = VOID_T;
	ptr_t.pointer = true;	
}

/* Given category from token, returns enum type */
enum type returnType(int type) {
	switch(type) {
		case INTEGER:
		case INT:
		case SHORT:
		case LONG:
			return INT_T;
		case FLOAT:
		case DOUBLE:
		case FLOATING:
			return FLOAT_T;
		case CHARACTER:
		case CHAR:
			return CHAR_T;
		case BOOL:
			return BOOL_T;
		case STRING:
			return STRING_T;
		case CLASS_NAME:
		case CLASS:
			return CLASS_T;
		case VOID:
			return VOID_T;
		default:
			return UNKNOWN_T;
	}
}

/* Allocates type for variables and classes */
struct typeinfo *alctype(struct tree *t){
	struct typeinfo *type = malloc(sizeof(struct typeinfo));
	if(t->kids[0]->nkids == -1)	
		type->basetype = returnType(t->kids[0]->leaf->category);
	struct tree *node = t;
	while(node->nkids != -1) 
		node = node->kids[0];
	type->token = node->leaf;
	if(productionBefore(t, declarator, compound_statement) != NULL)
		type->pointer = true;
	else
		type->pointer = false;
	if(type->basetype == CLASS_T)
		type->class.type = t->kids[0]->leaf->text;
	if(type->basetype == STRING_T) {
		type->class.type = strdup("string");
		type->basetype = CLASS_T;
	}
	
	return type;
}

/* Allocates type for function */
struct typeinfo *alctypeArray(struct tree *t, struct typeinfo *type) {
	
	struct typeinfo *array = alctype(t);
	array->basetype = ARRAY_T;
	array->array.type = type;
	struct tree *temp = getProduction(t, primary_expression_1);
	
	if (temp != NULL)
		array->array.size = atoi(temp->kids[0]->leaf->text);
	else
		array->array.size = 0;
	return array;
}

/* Allocates type for array */
struct typeinfo *alctypeFunction(struct tree *t, struct typeinfo *type) {
	struct typeinfo *function = alctype(t);
	function->basetype = FUNCTION_T;
	function->pointer = false;
	function->function.returntype = type;
	function->function.parameters = NULL;
	return function;
}

/* Returns copy of given type */
struct typeinfo *typeinfoCopy(struct typeinfo *type) {
	struct typeinfo *copy = malloc(sizeof(struct typeinfo));
	memcpy(copy, type, sizeof(*copy));
	return copy;
}

/* Compares lists. Used to check function parameter types */
bool compareLists(struct list *l, struct list *r) {
	while(l != NULL && r != NULL) {
		if(!typeinfoCompare(l->data, r->data))
			return false;
	l = l->next;
	r = r->next;
	}
	return true;
}

/* Compares types. Checks for pointers, array size, etc. */
bool typeinfoCompare(struct typeinfo *l, struct typeinfo *r){
	if (l == NULL && r == NULL)
		return true;

	if (l == NULL || r == NULL)
		return false; 

	if (l->basetype != r->basetype)
		return false;

	if (l->pointer != r->pointer)
		return false; 
	
	switch(l->basetype) {
		case ARRAY_T:
			if(!typeinfoCompare(l->array.type, r->array.type))
				return false;
			if(l->array.size != 0 && l->array.size != r->array.size)
				return false;
		case FUNCTION_T:
			if(!typeinfoCompare(l->function.returntype, r->function.returntype))
				return false;
		case CLASS_T:
			return(strcmp(l->class.type, r->class.type) == 0);
		default:
			return true;
	}	
	return false;
}

/* Returns size of type */
int getTypeSize(struct typeinfo *type) {
	struct symbol *s;	
	if(type->pointer)
		return 8;
	switch(type->basetype) {
		case INT_T:
		case FLOAT_T:
			return 8;
		case CHAR_T:
		case BOOL_T:
			return 1;
		case ARRAY_T:
			return type->array.size * getTypeSize(type->array.type);
		case FUNCTION_T:
			return 0;
		case CLASS_T:
			s = getSymbol(type->class.type, current);
			return tableSize(s->table);
		default:
			return 0;
	}
}

struct typeinfo *getRight(struct tree *t) {
	struct tree *child = t->kids[2];
	struct typeinfo *type = typecheck(child);
	return type;
}

struct typeinfo *getLeft(struct tree *t) {
	struct tree *child = t->kids[0];
	struct typeinfo *type = typecheck(child);
	return type;
}

struct typeinfo *getMiddle(struct tree *t) {
	struct tree *child = t->kids[1];
	struct typeinfo *type = typecheck(child);
	return type;
}


/* Big typechecking function. All operations from 120++ should be supported */
struct typeinfo *typecheck(struct tree *t){
	int i;	
	if (t == NULL)
		return NULL; 

	switch(t->label){
		/* Handles init_list and initializers. Checks for <fstream> include too as
		   ofstream and ifstream inserted from iostream but no allowed unless <fstream>
		   seen. Also handles array being initialized to list */	
		case init_declarator: {
			struct tree *node = getProduction(t, direct_declarator_1);
			struct symbol *sym = getSymbol(node->kids[0]->leaf->text, current);
			struct typeinfo *l = sym->type;
			if(l->basetype == CLASS_T && ((strcmp(l->class.type, "ofstream") == 0) 
						|| (strcmp(l->class.type, "ifstream") == 0)))
				if(!saw_fstream && ((strcmp(sym->name, "cout") != 0) && (strcmp(sym->name, "cin") != 0) && (strcmp(sym->name, "endl") !=0)))
					semantic_error("<fstream> not included", t);
			struct tree *temp = NULL;
			if((temp = getProduction(t, initializer_list)) != NULL) {
				t = temp;
				if(l->basetype != ARRAY_T)
					semantic_error("operand being initialized to list not an array", node);
				struct list *init = NULL;
				populateList(t, &init);
				while (init != NULL) {
					if (!typeinfoCompare(l->array.type,init->data))
						semantic_error("operand types mismatched in initializer list for", node);
					if (paramNum(init) > l->array.size)
						semantic_error("array size exceeded for", node);
					init = init->next;
				}
				return l;
			}	
			else if ((temp = getProduction(t, initializer)) != NULL) {
				t = temp;
				
					
				struct typeinfo *r = getLeft(t);
				if(l->basetype == ARRAY_T) {
					if (typeinfoCompare(l->array.type,r))
						return l;
					else		
						semantic_error("cannot initialize mismatched types", t);
				}
				else if (typeinfoCompare(l,r))
					return l;
				else
					semantic_error("cannot initialize mismatched types", t);
			}
			else 
				return l;
		}
		case new_expression: {
			//Todo: allow for things like new int
			struct tree *typespec = getProduction(t, type_specifier_seq);
			if (typespec == NULL)
				semantic_error("missing type spec", t);
			struct symbol *class = getSymbol(typespec->kids[0]->leaf->text, current);
			struct typeinfo *type = class->type;
			type->pointer = true;
			if (type->basetype == CLASS_T) {
				struct symbol *sym = getSymbol(type->class.type, class->table);
				struct typeinfo *ctor = sym->type;
				struct list *params = NULL;
				struct tree *node = getProduction(t, expression_list);
				while(node && node->label == expression_list) {
					struct typeinfo *type = typecheck(node);
					alclistfront(type, &params);
					node = node->kids[0];
				}
				struct list *temp = ctor->function.parameters;
				if(paramNum(temp) != paramNum(params))
					semantic_error("invalid number of parameters for constructor", typespec);
				while(temp != NULL && params != NULL){
					if(!typeinfoCompare(params->data, temp->data))
						semantic_error("parameter types do not match constructor", typespec);
					temp = temp->next;
					params = params->next;	
				}		
						
			}
			return type;
		}
		case delete_expression: {
			struct typeinfo *type = getMiddle(t);
			
			if(!type->pointer)
				semantic_error("delete operator requires pointer", t);
			return NULL;
		}
		case assignment_expression: {
			if(getProduction(t->kids[0], primary_expression_4) == NULL)
				semantic_error("operand unassignable", t);
			struct typeinfo *l = getLeft(t);
			struct typeinfo *r = getRight(t);
					
			if (typeinfoCompare(l,r))
				return l;
			else if (l->basetype == CLASS_T && (strcmp(l->class.type, "string") == 0) 
				&& r->basetype == CHAR_T && r->pointer)
				return l;
			else
				semantic_error("cannot assign mismatched types", t);
		}
		case logical_or_expression:
		case logical_and_expression: {
			struct typeinfo *l = getLeft(t);
			struct typeinfo *r = getRight(t);
			if(!(typeinfoCompare(l, &int_t) || typeinfoCompare(l, &bool_t)))
				semantic_error("operand not of type int or bool", t);
			if(!(typeinfoCompare(r, &int_t) || typeinfoCompare(r, &bool_t))) 
				semantic_error("operand not of type int or bool", t);
			return &bool_t;
		}
		case inclusive_or_expression:
		case exclusive_or_expression:
		case and_expression:
		case conditional_expression:
		case pm_expression: {
			semantic_error("operation not supported in 120++", t);
		}
		case equality_eq:
		case equality_neq: {
			struct typeinfo *l = getLeft(t);
			struct typeinfo *r = getRight(t);
			if (!typeinfoCompare(l,r))
				semantic_error ("operand types mismatched", t);
			return &bool_t;
		}
		case relational_lt:
		case relational_gt:
		case relational_lteq:
		case relational_gteq: {
			struct typeinfo *l = getLeft(t);
			struct typeinfo *r = getRight(t);
			if(!typeinfoCompare(l,r))
				semantic_error("operand types mismatched", t);
			return &bool_t;
		}
		case shift_expression: {
			typecheck(t->kids[0]);
			typecheck(t->kids[2]);
			break;
		}
		case additive_expression:
		case multiplicative_expression:
		case div_expression:
		case sub_expression: {
			struct typeinfo *l = getLeft(t);
			struct typeinfo *r = getRight(t);
			if(!typeinfoCompare(l,r))
				semantic_error("operand types mismatched", t);
			return l;
		}
		case modulus_expression: {
			struct typeinfo *l = getLeft(t);
			struct typeinfo *r = getRight(t);
			if(!typeinfoCompare(l, &int_t) || !typeinfoCompare(r, &int_t))
				semantic_error("operand not an integer", t);
			return l;
		}
		case unary_minusminus:
		case unary_plusplus: {
			struct typeinfo *r = getRight(t);
			switch(t->kids[0]->leaf->category) {
				case '+':
				case '=':
					return r;
			case '!':
				return &bool_t;	
			}
		}
		case unary_sizeof:
			return &int_t;
		case unary_star: {
			struct typeinfo *type = getMiddle(t);
			
			if(!type->pointer)
				semantic_error("dereferencing non-pointer", t);
			
			struct typeinfo *copy = typeinfoCopy(type);
			copy->pointer = false;
			return copy;
		}
		case unary_address: {
			struct typeinfo *type = getMiddle(t);
			
			if(type->pointer)
				semantic_error("double pointers not supported", t);
			
			struct typeinfo *copy = typeinfoCopy(type);
			copy->pointer = true;
			return copy;
		}
		case shift_left: {
			int i;
			struct typeinfo *ret = NULL;
			for(i = 0; i < t->nkids; i++) {
				struct typeinfo *type = typecheck(t->kids[i]);
				if (i == 0) {
					if (!(type->basetype == CLASS_T && (strcmp(type->class.type, "ofstream") == 0)))
						semantic_error("leftmost operand not of type ofstream", t);
					ret = type;
				}
					 else if (!(typeinfoCompare(type, &int_t)
			             			|| typeinfoCompare(type, &float_t)
			            	 		|| typeinfoCompare(type, &bool_t)
			            	 		|| typeinfoCompare(type, &char_t)
			           	 		|| typeinfoCompare(type, &string_t)
			            	 		|| (type->basetype == CLASS_T && (strcmp(type->class.type, "string") == 0))
							|| (type->basetype == CLASS_T && (strcmp(type->class.type, "ofstream") == 0))))					
						semantic_error("shift operand not valid type", t->kids[i]);
			}
			return ret;
			
		}
		case shift_right: {
			int i;
			struct typeinfo *ret = NULL;
			for(i = 0; i < t->nkids; i++) {
				struct typeinfo *type = typecheck(t->kids[i]);
				if (i == 0) {
					if (!(type->basetype == CLASS_T && (strcmp(type->class.type, "ifstream") == 0)))
						semantic_error("leftmost operand not of type ifstream", t);
					ret = type;
				}
				else if (!(typeinfoCompare(type, &int_t)
			           	  || typeinfoCompare(type, &float_t)
			          	  || typeinfoCompare(type, &char_t)
			          	  || typeinfoCompare(type, &string_t)
			         	  || (type->basetype == CLASS_T && (strcmp(type->class.type, "string") == 0)))) 

					semantic_error("shift operand not valid type", t->kids[i]);
			}
			return ret;
			
		}
		/* Makes sure to set current table to class while typechecking class */
		case class_specifier: {
			struct symbol *class = getSymbol(getClass(t->kids[0]), current);
			current = class->table;
			for (i = 0; i < t->nkids; i++)
				typecheck(t->kids[i]);
			current = current->parent;
			return class->type;
			
		}
		case ctor_function_definition:
		case function_definition_1: {
			struct symbol *function;
			struct symTable *current_ = current;
			struct tree *node;
			if (getProduction(t, ctor_function_definition))	{
				/* First getSymbol grabs class. Second grabs actual constructor */
				function = getSymbol(getFunction(t->kids[0]), current);
				current = function->table;
				function = getSymbol(getFunction(t->kids[0]), current);
				current = function->table;
			}
			else if((node = getProduction(t, direct_declarator_6)) != NULL) {
				/* First getSymbol grabs class. Second grabs actual function */
				function = getSymbol(node->kids[0]->leaf->text, current);
				current = function->table;
				function = getSymbol(node->kids[2]->leaf->text, current);
				current = function->table;
				node = node->kids[2];
			}
			else {
				node = getProduction(t, direct_declarator_1);
				function = getSymbol(getFunction(t->kids[1]), current);
				current = function->table;
			}
			struct typeinfo *ret = function->type;
			struct tree *state = getProduction(t, return_statement);
			if(t->label == ctor_function_definition)
				ret = function->type->function.returntype;
			else if(state == NULL || state->nkids == 2) 
				ret = &void_t;
			else 
				ret = typecheck(state->kids[1]);
			if(!typeinfoCompare(ret, function->type->function.returntype) && !typeinfoCompare(ret, &void_t))
				semantic_error("wrong return value type for function", node);
			for (i = 0; i < t->nkids; i++)
				typecheck(t->kids[i]);
			current = current_;

			return function->type->function.returntype;
		}
		/* Array access */
		case postfix_expression_1: {
			struct typeinfo *l = getLeft(t);
			struct typeinfo *r = getRight(t);
			if(l->basetype != ARRAY_T)
				semantic_error("variable that is not an array accesed as an array", t);
			if(!typeinfoCompare(&int_t, r))
				semantic_error("array index not an int", t->kids[2]);
			return l->array.type;
		}
		/* Function call */
		case postfix_expression_2: {
			struct typeinfo *l = getLeft(t);
			struct list *params = NULL;
			struct tree *node = getProduction(t, expression_list);
			while(node && node->label == expression_list) {
				struct typeinfo *type = typecheck(node);
				alclistfront(type, &params);
				node = node->kids[0];
			}
			if(paramNum(l->function.parameters) != paramNum(params)) {
				if (t->kids[0]->label == postfix_dot_expression || t->kids[0]->label == postfix_arrow_expression)
					semantic_error("invalid number of parameters for object", t);
				else
					semantic_error("invalid number of parameters", t);
			}
			if(!compareLists(l->function.parameters, params)) {
				if (t->kids[0]->label == postfix_dot_expression || t->kids[0]->label == postfix_arrow_expression)
					semantic_error("function call parameters types do not match definition for object", t);
				else
					semantic_error("function call parameters types do not match", t);
			}
			return l->function.returntype;
		}	
		case postfix_dot_expression: {
			struct typeinfo *l = getLeft(t);
			if (l->basetype != CLASS_T || l->pointer)
				semantic_error("expected variable to be a class instance", t);
			struct symbol *class = getSymbol(l->class.type, global);
			struct symbol *member = getSymbol(t->kids[2]->leaf->text, class->table);
			if(member == NULL)
				semantic_error("member not found in class. member:", t->kids[2]);
			return member->type;
		}
		case postfix_arrow_expression: {
			struct typeinfo *l = getLeft(t);
			if (l->basetype != CLASS_T || !l->pointer)
				semantic_error("expected variable to be a class pointer", t);
			struct symbol *class = getSymbol(l->class.type, current);
			struct symbol *member = getSymbol(t->kids[2]->leaf->text, class->table);
			if(member == NULL)
				semantic_error("member not found in class. member:", t->kids[2]);
			return member->type;
		}
		case expression_list: {
			struct tree *temp = t->kids[0];
			if (temp && temp->label == expression_list)
				return typecheck(t->kids[1]);
			else
				return typecheck(t->kids[0]);
		}	
		case primary_expression_4:
			if(getSymbol(t->kids[0]->leaf->text, current) == NULL){
				semantic_error("variable undeclared", t);
				return NULL;
			}
			else {
				struct symbol *temp = getSymbol(t->kids[0]->leaf->text, current);
				if(temp->funcDecl == true)
					semantic_error("function declared but not defined", t);
				return temp->type;
			}
		case primary_expression_1: {
			return alctype(t);
		}
		default: {
			for (i = 0; i < t->nkids; i++)
				typecheck(t->kids[i]);
		}
	}
	return NULL;
	
}



/* Prints given type */
void printType(enum type type) {
	switch(type) {
		case INT_T:
			printf("Type: int\n");
			break;
		case FLOAT_T:
			printf("Type: float\n");
			break;
		case CHAR_T:
			printf("Type: char\n");
			break;
		case ARRAY_T:
			printf("Type: array\n");
			break;
		case BOOL_T:
			printf("Type: bool\n");
			break;
		case CLASS_T:
			printf("Type: class\n");
			break;
		case VOID_T:
			printf("Type: void\n");
			break;
		case UNKNOWN_T:
			printf("Type: unknown\n");
			break;
		case FUNCTION_T:
			printf("Type: function\n");
			break;
		default:
			printf("No type assigned\n");
	}
}


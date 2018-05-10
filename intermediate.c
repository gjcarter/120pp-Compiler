#include <stdlib.h>
#include <stdio.h>
#include "intermediate.h"
#include "type.h"
#include "symtable.h"
#include "list.h"
#include "tree.h"

extern struct typeinfo int_t;
extern struct typeinfo float_t;
extern struct typeinfo char_t;
extern struct typeinfo array_t;
extern struct typeinfo bool_t;
extern struct typeinfo class_t;
extern struct typeinfo void_t;
extern struct typeinfo unknown_t;
extern struct typeinfo string_t;
extern struct typeinfo ptr_t;
struct address empty = { UNKNOWN_R, 0, &unknown_t};
struct address inc = { CONST_R, 1, &int_t };
extern enum region region;
extern int offset;
extern int labels;

struct op o_break;
struct op o_continue;

void gen(struct tree *t) {
	int offset_ = offset;
	struct symTable *current_ = current;
	int i;
	if (t == NULL)
		return;

	switch(t->label) {
		case class_specifier: {
			struct symbol *class = getSymbol(getClass(t->kids[0]), current);
			current = class->table;
			for (i = 0; i < t->nkids; i++)
				gen(t->kids[i]);
			current = current->parent;
			break;
		}
		case ctor_function_definition:
		case function_definition_1: {
			struct symbol *function;
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
			offset = tableSize(function->table);
			for (i = 0; i < t->nkids; i++)
				gen(t->kids[i]);
			break;
		}
		default:
			for (i = 0; i < t->nkids; i++)
				gen(t->kids[i]);
				break;
	}
	
	switch(t->label) {
		case initializer:
		case primary_expression_1:
		case primary_expression_4: {
			t->place = getAddress(t, 0);
			t->code = listconcat(t->code, t->kids[0]->code);
			break;
		}
		case init_declarator:{
			t->place = getAddress(t, 0);
			struct tree *node = getNode(t, 1);
			if(node == NULL)
				break;
			t->code = listconcat(t->code, t->kids[1]->code);
			alclistback(alctac(O_ASN, t->place, node->place, empty, NULL), &t->code);
			break;
		}			
		case additive_expression:
		case multiplicative_expression:
		case div_expression:
		case sub_expression:
		case modulus_expression: {
			struct address l = getAddress(t, 0);
			t->code = listconcat(t->code, t->kids[0]->code);
			struct address r = getAddress(t, 2);
			t->code = listconcat(t->code, t->kids[2]->code);
			t->place = newtemp(l.type);
			alclistback(alctac(getOpcode(t->label), t->place, l, r, NULL), &t->code);
			break;
		}
		case relational_lt:
		case relational_gt:
		case relational_lteq:
		case relational_gteq:
		case equality_eq:
		case equality_neq:
		case logical_or_expression:
		case logical_and_expression: {
			t->place = newtemp(&bool_t);
			struct address l = getAddress(t, 0);
			t->code = listconcat(t->code, t->kids[0]->code);
			struct address r = getAddress(t, 2);
			alclistback(alctac(getOpcode(t->label), t->place, l, r, NULL), &t->code);
			break;
	
		}
		case unary_plusplus:
		case unary_minusminus: {
			t->place = getAddress(t, 1);
			alclistback(alctac(getOpcode(t->label), t->place, t->place, inc, NULL), &t->code);
			break;
		}
		case unary_address: {
			t->place = newtemp(&ptr_t);
			t->code = listconcat(t->code, t->kids[1]->code);
			alclistback(alctac(O_ADDR, t->place, getAddress(t, 1), empty, NULL), &t->code);
			break;
		}
		
		case postfix_plusplus:
		case postfix_minusminus: {
			t->place = getAddress(t, 0);
			alclistback(alctac(getOpcode(t->label), t->place, t->place, inc, NULL), &t->code);
			break;
		}
		/* Array access */
		case postfix_expression_1: {
			struct tree *node = getNode(t, 0);
			struct symbol *s = getSymbol(node->kids[0]->leaf->text, current);
			struct address index = getAddress(t, 2);
			struct address offset = newtemp(&int_t);
			struct address size = { CONST_R, getTypeSize(s->type->array.type), &int_t };
			alclistback(alctac(O_MUL, offset, index, size, NULL), &t->code);
			struct address access = newtemp(s->type->array.type);
			alclistback(alctac(O_ADDR, access, s->type->place, empty, NULL), &t->code);
			alclistback(alctac(O_ADD, access, access, offset, NULL), &t->code);	
			t->place = newtemp(s->type->array.type);
			alclistback(alctac(O_DEREF, t->place, access, empty, NULL), &t->code);		
			break;
		}
		/* Function call */
		case postfix_expression_2: {
			struct tree *node = getNode(t, 0);
			struct symbol *s;
			if(node->label == postfix_dot_expression || node->label == postfix_dot_expression) {
				struct symbol *class;
				s = getSymbol(node->kids[0]->kids[0]->leaf->text, current);				
				class = getSymbol(s->type->class.type, global);
				s = getSymbol(node->kids[2]->leaf->text, class->table);
				struct address pointer;
				if(s->type->pointer)
					pointer = s->type->place;
				else {
					pointer = newtemp(&ptr_t);
					alclistback(alctac(O_ADDR, pointer, s->type->place , empty, s->name), &t->code);
				}
				alclistback(alctac(O_PARM, pointer, empty , empty, s->name), &t->code);
				
			}
			else
				s = getSymbol(node->kids[0]->leaf->text, current);
			if(typeinfoCompare(s->type->function.returntype, &void_t))
				t->place = empty;
			else
				t->place = newtemp(s->type->function.returntype);
			struct address count = { PARAM_R, paramNum(s->type->function.parameters), &int_t };
			t->code = listconcat(t->code, t->kids[2]->code);
			if(s->table)
				alclistback(alctac(O_CALL, count, t->place , empty, s->name), &t->code);
			else	
				alclistback(alctac(O_CALLC, count, t->place, empty, s->name), &t->code);
			break;
		}
		/* Parameter list */
		case expression_list: {
			for(i = 0; i < t->nkids; i++) {
				t->code = listconcat(t->code, t->kids[i]->code);
				struct address place = getAddress(t->kids[i], -1);
				if(place.region != UNKNOWN_R)
					alclistback(alctac(O_PARM, place, empty , empty, NULL), &t->code);
			}
			break;
		}
		case assignment_expression: {
			t->place = getAddress(t, 0);
			struct address r = getAddress(t, 2);
			t->code = listconcat(t->code, t->kids[0]->code);
			t->code = listconcat(t->code, t->kids[2]->code);
			alclistback(alctac(O_ASN, t->place, r, empty, NULL), &t->code);
			break;
		}
		case postfix_dot_expression:
		case postfix_arrow_expression:
		case delete_expression:
		case new_expression:
		case ctor_function_definition:
			break;
		case simple_declaration:
			break;
		case while_statement: {
			struct op *first = newlabel();
			struct op *body = newlabel();
			struct op *follow = newlabel();
			alclistback(first, &t->code);
			t->code = listconcat(t->code, t->kids[1]->code);
			alclistback(alctac(O_BIF, getAddress(t, 1), body->address[0] , empty, NULL), &t->code);
			alclistback(alctac(O_GOTO, follow->address[0], empty , empty, NULL), &t->code);
			alclistback(body, &t->code);
			statements(t->kids[2]->code, first, follow);
			t->code = listconcat(t->code, t->kids[2]->code);
			alclistback(alctac(O_GOTO, first->address[0], empty , empty, NULL), &t->code);
			alclistback(follow, &t->code);
			break;

		}
		case do_statement: {
			struct op *first = newlabel();
			struct op *follow = newlabel();
			alclistback(first, &t->code);
			statements(t->kids[1]->code, first, follow);
			t->code = listconcat(t->code, t->kids[1]->code);
			t->code = listconcat(t->code, t->kids[3]->code);
			alclistback(alctac(O_BIF, getAddress(t, 3), first->address[0] , empty, NULL), &t->code);
			alclistback(follow, &t->code);
			break;
		}
		case if_statement: {
			struct op *first = newlabel();
			struct op *follow = newlabel();
			t->code = listconcat(t->code, t->kids[1]->code);
			alclistback(alctac(O_BIF, getAddress(t, 1), first->address[0] , empty, NULL), &t->code);
			alclistback(alctac(O_GOTO, follow->address[0], empty , empty, NULL), &t->code);
			alclistback(first, &t->code);
			t->code = listconcat(t->code, t->kids[2]->code); 
			alclistback(follow, &t->code);
			break;
		}
		case ifelse_statement: {
			struct op *first = newlabel();
			struct op *follow = newlabel();
			t->code = listconcat(t->code, t->kids[1]->code);
			alclistback(alctac(O_BIF, getAddress(t, 1), first->address[0] , empty, NULL), &t->code);
			alclistback(alctac(O_GOTO, follow->address[0], empty , empty, NULL), &t->code);
			alclistback(first, &t->code);
			t->code = listconcat(t->code, t->kids[2]->code); 
			alclistback(follow, &t->code);
			t->code = listconcat(t->code, t->kids[4]->code); 
			break;
		}
		case for_statement: {
			t->code = listconcat(t->code, t->kids[1]->code);
			struct op *first = newlabel();
			struct op *body = newlabel();
			struct op *follow = newlabel();
			alclistback(first, &t->code);
			t->code = listconcat(t->code, t->kids[2]->code);
			alclistback(alctac(O_BIF, getAddress(t, 2), body->address[0] , empty, NULL), &t->code);
			alclistback(alctac(O_GOTO, follow->address[0], empty , empty, NULL), &t->code);	
			alclistback(body, &t->code);
			statements(t->kids[4]->code, first, follow);
			t->code = listconcat(t->code, t->kids[4]->code);
			t->code = listconcat(t->code, t->kids[3]->code);
			alclistback(alctac(O_GOTO, first->address[0], empty, empty, NULL), &t->code);
			alclistback(follow, &t->code);
			break;
		}
		case break_statement:
			alclistback(&o_break, &t->code);
			break;
		case continue_statement:
			alclistback(&o_continue, &t->code);
			break;
		case return_statement: {
			if(getNode(t, 1)) {
				t->place = getAddress(t, 1);
				t->code = listconcat(t->code, t->kids[1]->code);
				alclistback(alctac(O_RET, t->place, empty , empty, NULL), &t->code);
			}
			else
				alclistback(alctac(O_RET, empty, empty , empty, NULL), &t->code);
			break;
		}
		case function_definition_1: {
			struct symbol *function;
			struct tree *node;
			if((node = getProduction(t, direct_declarator_6)) != NULL) 
				function = getSymbol(node->kids[2]->leaf->text, current);
			else 
				function = getSymbol(getFunction(t->kids[1]), current);
			struct address parameters = { PARAM_R, function->type->function.paramSize, &int_t };
			struct address local = { LOCAL_R, offset, &int_t };
			struct address ret = { UNKNOWN_R, 0, function->type };
			
			alclistback(alctac(O_PROC, parameters, local, ret, function->name), &t->code);	
			t->code = listconcat(t->code, t->kids[3]->code);
			alclistback(alctac(O_END, empty, empty, empty, NULL), &t->code);
			current = current_;
			offset = offset_;
			break;
		}
		
		default:
			for (i = 0; i < t->nkids; i++) {
				if(t->kids[i] != NULL)
					t->code = listconcat(t->code, t->kids[i]->code);
			}
			break;
				
				
				
				
	}
		
}

struct address newtemp(struct typeinfo *type){
	struct address temp = { current->scope, offset, type };
	offset += getTypeSize(type);
	return temp;
}

struct op *newlabel() {
	struct address place = { LABEL_R, labels++, &unknown_t };
	return alctac(O_LABEL, place, empty, empty, NULL);
}

void statements(struct list *code, struct op *first, struct op *follow) {
	struct list *temp = code;
	while(temp != NULL) {
		struct op *op = temp->data;
		if(first && op == &o_continue)
			temp->data = alctac(O_GOTO, first->address[0], empty, empty, NULL);
		if(follow && op == &o_break)
			temp->data = alctac(O_GOTO, follow->address[0], empty, empty, NULL);
		temp = temp->next;
	}
}

enum opcode getOpcode(int label) {
	
	switch(label) {	
		case unary_plusplus:
		case additive_expression:
		case postfix_plusplus:
			return O_ADD;
		case multiplicative_expression:
			return O_MUL;
		case div_expression:
			return O_DIV;
		case unary_minusminus:
		case sub_expression:
		case postfix_minusminus:
			return O_SUB;
		case relational_lt:
			return O_BLT;
		case relational_gt:
			return O_BGT;
		case relational_lteq:
			return O_BLE;
		case relational_gteq:
			return O_BGE;
		case equality_eq:
			return O_BEQ;
		case equality_neq:
			return O_BNE;
		case logical_or_expression:
			return O_OR;
		case logical_and_expression:
			return O_AND;
		default:
			return O_ERR;
	}
	
}

struct address getAddress(struct tree *t, int child){
	struct tree *node;
	if(child == -1)
		 node = t;
	else
		node = getNode(t, child);
	if(node) {
		if(node->place.region != UNKNOWN_R) 
			return node->place;
		
		if(node->label == 0) {
			struct symbol *s = getSymbol(node->leaf->text, current);
			if(!s)
				s = getSymbol(node->leaf->text, constant);
			if(s)
				if(s->type->place.region != UNKNOWN_R) 
					return s->type->place;	
		
		}
	}
	
	return empty;
		
}

struct op *alctac(enum opcode code, struct address a, struct address b, struct address c, char *name) {
	struct op *op = malloc(sizeof(struct op));
	
	op->opcode = code;
	op->address[0] = a;
	op->address[1] = b;
	op->address[2] = c;
	op->name = name;
	return op;
}

void addtac(struct tree *t, struct op *op) {
	alclistback(op, &t->code);
}

void printaddress(FILE *file, struct address address){
	if(address.region == PARAM_R)
		fprintf(file, "%d", address.offset);
	else
		fprintf(file, "%s: %d", regionstring(address.region), address.offset);
}

char *regionstring(enum region region) {
	switch(region) {
		case GLOBAL_R:
			return "global";
		case LOCAL_R:
			return "local";
		case CLASS_R:
			return "class";
		case UNKNOWN_R:
			return "unknown";
		case CONST_R:
			return "const";
		case PARAM_R:
			return " ";
		case LABEL_R:
			return "label";
	}
	return NULL;
}

char *opcodestring(enum opcode code) {
	switch(code) {
		case O_ADD:
			return "add";
		case O_SUB:
			return "sub";
		case O_MUL:
			return "mul";
		case O_DIV:
			return "div";
		case O_NEG:
			return "neg";
		case O_ASN:
			return "asn";
		case O_ADDR:
			return "addr";
		case O_LCONT:
			return "lcont";
		case O_SCONT:
			return "rcont";
		case O_GOTO:
			return "goto";
		case O_BLT:
			return "blt";
		case O_BLE:
			return "ble";
		case O_BGT:
			return "bgt";
		case O_BGE:
			return "bge";
		case O_BEQ:
			return "beq";
		case O_BNE:
			return "bne";
		case O_BIF:
			return "bif";
		case O_PARM:
			return "parm";
		case O_CALL:
			return "call";
		case O_RET:
			return "ret";
		case O_GLOB:
			return "global";
		case O_PROC:
			return "proc";
		case O_LOCAL:
			return "loc";
		case O_LABEL:
			return "lab";
		case O_END:
			return "end";
		case O_OR:
			return "or";
		case O_AND:
			return "and";
		case O_DEREF:
			return "deref";
		case O_CALLC:
			return "callc";
		default:
			return "unknown instr";
	}
	return NULL;
}

void printCodeList(FILE *file, struct list *code) {
	struct list *temp = code;
	int i;
	while(temp != NULL) {
		struct op *op = ((struct op *)temp->data);
		if(op->opcode == O_PROC)
			fprintf(file, "%-s ", opcodestring(op->opcode));
		else
			fprintf(file, "%10s", " ");
		if(op->opcode != O_PROC)
			fprintf(file, "%s", opcodestring(op->opcode));
		if(op->name)
			fprintf(file, " %-s,", op->name);
		else
			fprintf(file, " ");
		for(i = 0; i < 3; i++) {
			struct address address = op->address[i];
			if(address.region != UNKNOWN_R) {
				printaddress(file, address);
				if(i < 2)
					fprintf(file, ",");
			}
		}
		fprintf(file, "\n");
		temp = temp->next;
	}
}

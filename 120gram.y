/*
 * Grammar for 120++, a subset of C++ used in CS 120 at University of Idaho
 *
 * Adaptation by Clinton Jeffery, with help from Matthew Brown, Ranger
 * Adams, and Shea Newton.
 *
 * Based on Sandro Sigala's transcription of the ISO C++ 1996 draft standard.
 * 
 */

/*	$Id: parser.y,v 1.3 1997/11/19 15:13:16 sandro Exp $	*/

/*
 * Copyright (c) 1997 Sandro Sigala <ssigala@globalnet.it>.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
 * OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
 * IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
 * NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
 * THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

/*
 * ISO C++ parser.
 *
 * Based on the ISO C++ draft standard of December '96.
 */


/* Rules that aren't part of 120++ have the action 
 * { $$ = NULL; yyerror("C++ operation not supported"); }
 * Not all rules that aren't part of 120++ have this action as 
 * some of them are terminals of the others and end up reaching
 * the error eventually. Nonterminals that aren't in 120++ are
 * set to NULL and error handling is handled in the lex file
*/

%{
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "nonterm.h"
#include "tree.h"
#include "token.h"



#define YYDEBUG 1
int yydebug=0;
static void yyerror(char *s);
extern FILE *yyin;
extern FILE *yyout;
extern char *yytext;
extern int yylex(void);
extern int lineno;
extern char* filename;
extern int typenametable_insert(char *s, int cat);
//Root for entire tree for file
struct tree *root;


%}
%union {
  struct tree *treenode; 
  }
  
%type < treenode > direct_declarator parameter_declaration_clause
%type < treenode > parameter_declaration_list parameter_declaration id_expression
%type < treenode > storage_class_specifier type_specifier simple_type_specifier 
%type < treenode > ptr_operator nested_name_specifier_opt
%type < treenode > declarator_id constant_expression_opt unqualified_id qualified_id
%type < treenode > cv_qualifier_seq exception_specification nested_name_specifier  
%type < treenode > operator_function_id conversion_function_id template_id class_name 
%type < treenode > typedef_name enum_name namespace_name original_namespace_name
%type < treenode > cv_qualifier decl_specifier_seq declarator assignment_expression
%type < treenode > abstract_declarator_opt logical_or_expression conditional_expression 
%type < treenode > expression constant_expression logical_and_expression 
%type < treenode > throw_expression inclusive_or_expression integer_literal 
%type < treenode > character_literal floating_literal string_literal boolean_literal
%type < treenode > literal primary_expression direct_new_declarator new_placement 
%type < treenode > postfix_expression expression_list expression_list_opt 
%type < treenode > unary_expression pm_expression multiplicative_expression 
%type < treenode > shift_expression relational_expression equality_expression 
%type < treenode > exclusive_or_expression delete_expression new_initializer
%type < treenode > type_id unary_operator new_expression new_type_id new_declarator
%type < treenode > labeled_statement assignment_operator compound_statement statement
%type < treenode > selection_statement iteration_statement declaration_statement 
%type < treenode > expression_opt statement_seq_opt condition for_init_statement 
%type < treenode > jump_statement simple_declaration block_declaration 
%type < treenode > template_declaration explicit_instantiation linkage_specification 
%type < treenode > namespace_definition asm_definition namespace_alias_definition
%type < treenode > using_declaration using_directive enumerator_definition enumerator
%type < treenode > named_namespace_definition original_namespace_definition  
%type < treenode > declaration_seq_opt init_declarator function_body 
%type < treenode > init_declarator_list initializer_clause type_specifier_seq 
%type < treenode > member_declarator constant_initializer new_placement_opt 
%type < treenode > declaration enumerator_list namespace_body conversion_declarator
%type < treenode > additive_expression new_declarator_opt decl_specifier type_name
%type < treenode > conversion_type_id and_expression try_block statement_seq
%type < treenode > function_definition cast_expression expression_statement
%type < treenode > member_declaration base_clause ctor_initializer
%type < treenode > mem_initializer_list template_parameter_list
%type < treenode > template_parameter template_argument_list base_specifier_list
%type < treenode > explicit_specialization exception_declaration
%type < treenode > type_id_list initializer initializer_list mem_initializer_id
%type < treenode > COLONCOLON_opt new_initializer_opt condition_opt
%type < treenode > enumerator_list_opt initializer_opt member_declarator_list
%type < treenode > type_specifier_seq_opt direct_abstract_declarator_opt 
%type < treenode > ctor_initializer_opt COMMA_opt member_specification_opt
%type < treenode > SEMICOLON_opt conversion_declarator_opt EXPORT_opt handler_seq_opt
%type < treenode > assignment_expression_opt type_id_list_opt operator
%type < treenode > type_parameter template_argument function_specifier 
%type < treenode > declaration_seq base_specifier mem_initializer 
%type < treenode > template_name abstract_declarator identifier program
%type < treenode > direct_abstract_declarator member_specification 
%type < treenode > handler_seq class_key pure_specifier access_specifier
%type < treenode > elaborated_type_specifier enum_specifier handler
%type < treenode > extension_namespace_definition unnamed_namespace_definition
%type < treenode > qualified_namespace_specifier function_try_block 
%type < treenode > class_specifier class_head

%token < treenode > IDENTIFIER INTEGER FLOATING CHARACTER STRING
%token < treenode > TYPEDEF_NAME NAMESPACE_NAME CLASS_NAME ENUM_NAME TEMPLATE_NAME
%token < treenode > ELLIPSIS COLONCOLON DOTSTAR ADDEQ SUBEQ MULEQ DIVEQ MODEQ
%token < treenode > XOREQ ANDEQ OREQ SL SR SREQ SLEQ EQ NOTEQ LTEQ GTEQ ANDAND OROR
%token < treenode > PLUSPLUS MINUSMINUS ARROWSTAR ARROW

%token < treenode > ASM AUTO BOOL BREAK CASE CATCH CHAR CLASS CONST CONST_CAST CONTINUE
%token < treenode > DEFAULT DELETE DO DOUBLE DYNAMIC_CAST ELSE ENUM EXPLICIT EXPORT EXTERN
%token < treenode > FALSE FLOAT FOR FRIEND GOTO IF INLINE INT LONG MUTABLE NAMESPACE NEW
%token < treenode > OPERATOR PRIVATE PROTECTED PUBLIC REGISTER REINTERPRET_CAST RETURN
%token < treenode > SHORT SIGNED SIZEOF STATIC STATIC_CAST STRUCT SWITCH TEMPLATE THIS
%token < treenode > THROW TRUE TRY TYPEDEF TYPEID TYPENAME UNION UNSIGNED USING VIRTUAL
%token < treenode > VOID VOLATILE WCHAR_T WHILE
%token < treenode > '~' '=' ',' '?' ':' '(' ')' '.' '<' '>' '*' '&' '0'
%token < treenode > '+' '-' '^' '|' '{' '}' '[' ']' '/' '%' ';' '!' '_'

%start program

%%


/*----------------------------------------------------------------------
 * Context-dependent identifiers.
 *----------------------------------------------------------------------*/

typedef_name:
	/* identifier */
	TYPEDEF_NAME { $$ = NULL;}
	;

namespace_name:
	original_namespace_name { $$ = NULL; }
	;

original_namespace_name:
	/* identifier */
	NAMESPACE_NAME { $$ = NULL;}
	;

class_name:
	/* identifier */
	CLASS_NAME { $$ = $1;}
	| template_id { $$ = NULL; }
	;

enum_name:
	/* identifier */
	ENUM_NAME { $$ = NULL;}
	;

template_name:
	/* identifier */
	TEMPLATE_NAME { $$ = NULL;}
	;

/*----------------------------------------------------------------------
 * Lexical elements.
 *----------------------------------------------------------------------*/

identifier:
	IDENTIFIER { $$ = $1;}
	;

literal:
	integer_literal { $$ = $1; }
	| character_literal { $$ = $1; }
	| floating_literal { $$ = $1; }
	| string_literal { $$ = $1; }
	| boolean_literal { $$ = $1; }
	;

integer_literal:
	INTEGER { $$ = $1;}
	;

character_literal:
	CHARACTER { $$ = $1;}
	;

floating_literal:
	FLOATING { $$ = $1;}
	;

string_literal:
	STRING { $$ = $1;}
	;

boolean_literal:
	TRUE { $$ = $1;}
	| FALSE { $$ = $1;}
	;

/*----------------------------------------------------------------------
 * Program start.
 *----------------------------------------------------------------------*/

program:
	declaration_seq_opt { $$ = $1; root = $$; }
	

/*----------------------------------------------------------------------
 * Expressions.
 *----------------------------------------------------------------------*/

primary_expression:
	literal { $$ = alctree(primary_expression_1, 1, $1); }
	| THIS { $$ = $1;}
	| '(' expression ')' { $$ = $2; }
	| id_expression { $$ = alctree(primary_expression_4, 1, $1); }
	;

id_expression:
	unqualified_id { $$ = $1; }
	| qualified_id { $$ = $1; }
	;

unqualified_id:
	identifier { $$ = $1; }
	| operator_function_id  { $$ = $1; }
	| conversion_function_id { $$ = $1; }
	| '~' class_name { $$ = alctree(unqualified_id, 2, $1, $2); }
	;

qualified_id:
	nested_name_specifier unqualified_id { $$ = alctree(qualified_id, 2, $1, $2); }
	| nested_name_specifier TEMPLATE unqualified_id { $$ = NULL; yyerror("C++ operation not supported"); }
	;

nested_name_specifier:
	class_name COLONCOLON nested_name_specifier { $$ = alctree(nested_name_specifier, 2, $1, $2); }
	| namespace_name COLONCOLON nested_name_specifier { $$ = alctree(nested_name_specifier, 3, $1, $2, $3); }
	| class_name COLONCOLON { $$ = alctree(nested_name_specifier, 2, $1, $2); }
	| namespace_name COLONCOLON { $$ = alctree(nested_name_specifier, 2, $1, $2); }
	;

postfix_expression:
	primary_expression { $$ = $1; }
	| postfix_expression '[' expression ']' { $$ = alctree(postfix_expression_1, 4, $1, $2, $3, $4); }
	| postfix_expression '(' expression_list_opt ')' { $$ = alctree(postfix_expression_2, 4, $1, $2, $3, $4); }
	| DOUBLE '(' expression_list_opt ')' { $$ = alctree(postfix_expression, 4, $1, $2, $3, $4); }
	| INT '(' expression_list_opt ')' { $$ = alctree(postfix_expression, 4, $1, $2, $3, $4); }
	| CHAR '(' expression_list_opt ')' { $$ = alctree(postfix_expression, 4, $1, $2, $3, $4); }
	| BOOL '(' expression_list_opt ')' { $$ = alctree(postfix_expression, 4, $1, $2, $3, $4); }
	| postfix_expression '.' TEMPLATE COLONCOLON id_expression { $$ = NULL; yyerror("C++ operation not supported"); }
	| postfix_expression '.' TEMPLATE id_expression { $$ = NULL; yyerror("C++ operation not supported"); }
	| postfix_expression '.' COLONCOLON id_expression { $$ = alctree(postfix_expression, 4, $1, $2, $3, $4); }
	| postfix_expression '.' id_expression { $$ = alctree(postfix_dot_expression, 3, $1, $2, $3); }
	| postfix_expression ARROW TEMPLATE COLONCOLON id_expression { $$ = NULL; yyerror("C++ operation not supported"); }
	| postfix_expression ARROW TEMPLATE id_expression { $$ = NULL; yyerror("C++ operation not supported"); }
	| postfix_expression ARROW COLONCOLON id_expression { $$ = alctree(postfix_expression, 4, $1, $2, $3, $4); }
	| postfix_expression ARROW id_expression { $$ = alctree(postfix_arrow_expression, 3, $1, $2, $3); }
	| postfix_expression PLUSPLUS { $$ = alctree(postfix_plusplus, 2, $1, $2); }
	| postfix_expression MINUSMINUS { $$ = alctree(postfix_minusminus, 2, $1, $2); }
	| DYNAMIC_CAST '<' type_id '>' '(' expression ')' { $$ = NULL; yyerror("C++ operation not supported"); }
	| STATIC_CAST '<' type_id '>' '(' expression ')' { $$ = NULL; yyerror("C++ operation not supported"); }
	| REINTERPRET_CAST '<' type_id '>' '(' expression ')' { $$ = NULL; yyerror("C++ operation not supported"); }
	| CONST_CAST '<' type_id '>' '(' expression ')' { $$ = NULL; yyerror("C++ operation not supported"); }
	| TYPEID '(' expression ')' { $$ = NULL; yyerror("C++ operation not supported"); }
	| TYPEID '(' type_id ')' { $$ = NULL; yyerror("C++ operation not supported"); }
	;

expression_list:
	assignment_expression {$$ = alctree(expression_list, 1, $1); }
	| expression_list ',' assignment_expression { $$ = alctree(expression_list, 2, $1, $3); }
	;

unary_expression:
	postfix_expression { $$ = $1; }
	| PLUSPLUS cast_expression { $$ = alctree(unary_plusplus, 2, $1, $2); }
	| MINUSMINUS cast_expression { $$ = alctree(unary_minusminus, 2, $1, $2); }
	| '*' cast_expression { $$ = alctree(unary_star, 2, $1, $2); }
	| '&' cast_expression { $$ = alctree(unary_address, 2, $1, $2); }
	| unary_operator cast_expression { $$ = alctree(unary_op, 2, $1, $2); }
	| SIZEOF unary_expression { $$ = alctree(unary_sizeof, 2, $1, $2); }
	| SIZEOF '(' type_id ')' { $$ = alctree(unary_sizeof, 4, $1, $2, $3, $4); }
	| new_expression { $$ = $1; }
	| delete_expression { $$ = $1; }
	;

unary_operator:
	  '+' { $$ = $1;}
	| '-' { $$ = $1;}
	| '!' { $$ = $1;}
	| '~' { $$ = $1;}
	;

new_expression:
	  NEW new_placement_opt new_type_id new_initializer_opt { $$ = alctree(new_expression, 4, $1, $2, $3, $4); }
	| COLONCOLON NEW new_placement_opt new_type_id new_initializer_opt { $$ = alctree(new_expression, 5, $1, $2, $3, $4, $5); }
	| NEW new_placement_opt '(' type_id ')' new_initializer_opt { $$ = alctree(new_expression, 6, $1, $2, $3, $4, $5, $6); }
	| COLONCOLON NEW new_placement_opt '(' type_id ')' new_initializer_opt { $$ = alctree(new_expression, 7, $1, $2, $3, $4, $5, $6, $7); }
	;

new_placement:
	'(' expression_list ')' { $$ = alctree(new_placement, 3, $1, $2, $3); }
	;

new_type_id:
	type_specifier_seq new_declarator_opt { $$ = alctree(new_type_id, 2, $1, $2); }
	;

new_declarator:
	ptr_operator new_declarator_opt { $$ = alctree(new_declarator, 2, $1, $2); }
	| direct_new_declarator { $$ = $1; }
	;

direct_new_declarator:
	'[' expression ']' { $$ = alctree(direct_new_declarator, 3, $1, $2, $3); }
	| direct_new_declarator '[' constant_expression ']' { $$ = alctree(direct_new_declarator, 4, $1, $2, $3, $4); }
	;

new_initializer:
	'(' expression_list_opt ')' { $$ = alctree(new_initializer, 3, $1, $2, $3); }
	;

delete_expression:
	  DELETE cast_expression { $$ = alctree(delete_expression, 2, $1, $2); }
	| COLONCOLON DELETE cast_expression { $$ = alctree(delete_expression, 3, $1, $2, $3); }
	| DELETE '[' ']' cast_expression { $$ = alctree(delete_expression, 4, $1, $2, $3, $4); }
	| COLONCOLON DELETE '[' ']' cast_expression { $$ = alctree(delete_expression, 4, $1, $2, $3, $4); }
	;

cast_expression:
	unary_expression { $$ = $1; }
	| '(' type_id ')' cast_expression { $$ = alctree(cast_expression, 4, $1, $2, $3, $4); }
	;

pm_expression:
	cast_expression { $$ = $1; }
	| pm_expression DOTSTAR cast_expression { $$ = alctree(pm_expression, 3, $1, $2, $3); }
	| pm_expression ARROWSTAR cast_expression { $$ = alctree(pm_expression, 3, $1, $2, $3); }
	;

multiplicative_expression:
	pm_expression { $$ = $1; }
	| multiplicative_expression '*' pm_expression { $$ = alctree(multiplicative_expression, 3, $1, $2, $3); }
	| multiplicative_expression '/' pm_expression { $$ = alctree(div_expression, 3, $1, $2, $3); }
	| multiplicative_expression '%' pm_expression { $$ = alctree(modulus_expression, 3, $1, $2, $3); }
	;

additive_expression:
	multiplicative_expression { $$ = $1; }
	| additive_expression '+' multiplicative_expression { $$ = alctree(additive_expression, 3, $1, $2, $3); }
	| additive_expression '-' multiplicative_expression { $$ = alctree(sub_expression, 3, $1, $2, $3); }
	;

shift_expression:
	additive_expression { $$ = $1; }
	| shift_expression SL additive_expression { $$ = alctree(shift_left, 2, $1, $3); }
	| shift_expression SR additive_expression { $$ = alctree(shift_right, 2, $1, $3); }
	;

relational_expression:
	shift_expression { $$ = $1; }
	| relational_expression '<' shift_expression { $$ = alctree(relational_lt, 3, $1, $2, $3); }
	| relational_expression '>' shift_expression { $$ = alctree(relational_gt, 3, $1, $2, $3); }
	| relational_expression LTEQ shift_expression { $$ = alctree(relational_lteq, 3, $1, $2, $3); }
	| relational_expression GTEQ shift_expression { $$ = alctree(relational_gteq, 3, $1, $2, $3); }
	;

equality_expression:
	relational_expression { $$ = $1; }
	| equality_expression EQ relational_expression { $$ = alctree(equality_eq, 3, $1, $2, $3); }
	| equality_expression NOTEQ relational_expression { $$ = alctree(equality_neq, 3, $1, $2, $3); }
	;

and_expression:
	equality_expression { $$ = $1; }
	| and_expression '&' equality_expression { $$ = alctree(and_expression, 3, $1, $2, $3); }
	;

exclusive_or_expression:
	and_expression { $$ = $1; }
	| exclusive_or_expression '^' and_expression { $$ = alctree(exclusive_or_expression, 3, $1, $2, $3); }
	;

inclusive_or_expression:
	exclusive_or_expression { $$ = $1; }
	| inclusive_or_expression '|' exclusive_or_expression { $$ = alctree(inclusive_or_expression, 3, $1, $2, $3); }
	;

logical_and_expression:
	inclusive_or_expression { $$ = $1; }
	| logical_and_expression ANDAND inclusive_or_expression { $$ = alctree(logical_and_expression, 3, $1, $2, $3); }
	;

logical_or_expression:
	logical_and_expression { $$ = $1; }
	| logical_or_expression OROR logical_and_expression { $$ = alctree(logical_or_expression, 3, $1, $2, $3); }
	;

conditional_expression:
	logical_or_expression { $$ = $1; }
	| logical_or_expression  '?' expression ':' assignment_expression { $$ = alctree(conditional_expression, 5, $1, $2, $3, $4, $5); }
	;

assignment_expression:
	conditional_expression { $$ = $1; }
	| logical_or_expression assignment_operator assignment_expression { $$ = alctree(assignment_expression, 3, $1, $2, $3); }
	| throw_expression { $$ = $1; }
	;

assignment_operator:
	'=' { $$ = $1;}
	| MULEQ { $$ = $1;}
	| DIVEQ { $$ = $1;}
	| MODEQ { $$ = $1;}
	| ADDEQ { $$ = $1;}
	| SUBEQ { $$ = $1;}
	| SREQ { $$ = NULL;}
	| SLEQ { $$ = NULL;}
	| ANDEQ { $$ = NULL;}
	| XOREQ { $$ = NULL;}
	| OREQ { $$ = NULL;}
	;

expression:
	assignment_expression { $$ = $1; }
	| expression ',' assignment_expression { $$ = alctree(expression, 3, $1, $2, $3); }
	;

constant_expression:
	conditional_expression { $$ = $1; }
	; 

/*----------------------------------------------------------------------
 * Statements.
 *----------------------------------------------------------------------*/

statement:
	labeled_statement { $$ = $1; }
	| expression_statement { $$ = $1; }
	| compound_statement { $$ = $1; }
	| selection_statement { $$ = $1; }
	| iteration_statement { $$ = $1; }
	| jump_statement { $$ = $1; }
	| declaration_statement { $$ = $1; }
	| try_block { $$ = $1; }
	;

labeled_statement:
	identifier ':' statement { $$ = alctree(labeled_statement, 2, $1, $3); }
	| CASE constant_expression ':' statement { $$ = alctree(case_statement, 3, $1, $2, $4); }
	| DEFAULT ':' statement { $$ = alctree(default_statement, 2, $1, $3); }
	;

expression_statement:
	expression_opt ';' { $$ = alctree(expression_statement, 2, $1, $2); }
	;

compound_statement:
	'{' statement_seq_opt '}' { $$ = alctree(compound_statement, 3, $1, $2, $3); }
	;

statement_seq:
	statement { $$ = $1; }
	| statement_seq statement { $$ = alctree(statement_seq, 2, $1, $2); }
	;

selection_statement:
	IF '(' condition ')' statement { $$ = alctree(if_statement, 3, $1, $3, $5); }
	| IF '(' condition ')' statement ELSE statement { $$ = alctree(ifelse_statement, 5, $1, $3, $5, $6, $7); }
	| SWITCH '(' condition ')' statement { $$ = alctree(switch_statement, 3, $1, $3, $5); }
	;

condition:
	expression { $$ = $1; }
	| type_specifier_seq declarator '=' assignment_expression { $$ = alctree(conditional_expression, 4, $1, $2, $3, $4); }
	;

iteration_statement:
	WHILE '(' condition ')' statement { $$ = alctree(while_statement, 3, $1, $3, $5); }
	| DO statement WHILE '(' expression ')' ';' { $$ = alctree(do_statement, 4, $1, $2, $3, $5); }
	| FOR '(' for_init_statement condition_opt ';' expression_opt ')' statement { $$ = alctree(for_statement, 5, $1, $3, $4, $6, $8); }
	;

for_init_statement:
	expression_statement { $$ = $1; }
	| simple_declaration { $$ = $1; }
	;

jump_statement:
	BREAK ';' { $$ = alctree(break_statement, 2, $1, $2); }
	| CONTINUE ';' { $$ = alctree(continue_statement, 2, $1, $2); }
	| RETURN expression_opt ';' { $$ = alctree(return_statement, 3, $1, $2, $3); }
	| GOTO identifier ';' { $$ = alctree(goto_statement, 3, $1, $2, $3); }
	;

declaration_statement:
	block_declaration { $$ = $1; }
	;

/*----------------------------------------------------------------------
 * Declarations.
 *----------------------------------------------------------------------*/

declaration_seq:
	declaration { $$ = $1; }
	| declaration_seq declaration { $$ = alctree(declaration_seq, 2, $1, $2); }
	;

declaration:
	block_declaration { $$ = $1; }
	| function_definition { $$ = $1; }
	| template_declaration { $$ = NULL; yyerror("C++ operation not supported"); }
	| explicit_instantiation { $$ = NULL; yyerror("C++ operation not supported"); }
	| explicit_specialization { $$ = NULL; yyerror("C++ operation not supported"); }
	| linkage_specification { $$ = NULL; yyerror("C++ operation not supported"); }
	| namespace_definition { $$ = NULL; yyerror("C++ operation not supported"); }
	; 

block_declaration:
	simple_declaration { $$ = $1; }
	| asm_definition { $$ = NULL; yyerror("C++ operation not supported"); }
	| namespace_alias_definition { $$ = NULL; yyerror("C++ operation not supported"); }
	| using_declaration { $$ = NULL; yyerror("C++ operation not supported"); }
	| using_directive { $$ = NULL; yyerror("C++ operation not supported"); }
	;

simple_declaration:
	  decl_specifier_seq init_declarator_list ';' { $$ = alctree(simple_declaration_1, 3, $1, $2, $3); }
	|  decl_specifier_seq ';' { $$ = alctree(simple_declaration, 2, $1, $2); }
	;

decl_specifier:
	storage_class_specifier { $$ = $1; }
	| type_specifier { $$ = $1; }
	| function_specifier { $$ = NULL; yyerror("C++ operation not supported"); }
	| FRIEND { $$ = NULL; }
	| TYPEDEF { $$ = NULL;}
	;

decl_specifier_seq:
	  decl_specifier { $$ = $1; }
	| decl_specifier_seq decl_specifier { $$ = alctree(decl_specifier_seq, 2, $1, $2); }
	;

storage_class_specifier:
	AUTO  { $$ = NULL; }
	| REGISTER { $$ = NULL; }
	| STATIC { $$ = NULL; }
	| EXTERN { $$ = NULL; }
	| MUTABLE { $$ = NULL; }
	;

function_specifier:
	INLINE  { $$ = NULL; }
	| VIRTUAL { $$ = NULL; }
	| EXPLICIT { $$ = NULL; }
	;

type_specifier:
	simple_type_specifier { $$ = $1; }
	| class_specifier { $$ = $1; }
	| enum_specifier { $$ = $1; }
	| elaborated_type_specifier { $$ = $1; }
	| cv_qualifier { $$ = $1; }
	;

simple_type_specifier:
	  type_name { $$ = $1; }
	| nested_name_specifier type_name { $$ = alctree(simple_type_specifier, 2, $1, $2); }
	| COLONCOLON nested_name_specifier_opt type_name { $$ = alctree(simple_type_specifier, 3, $1, $2, $3); }
	| CHAR { $$ = $1;}
	| WCHAR_T { $$ = $1;}
	| BOOL { $$ = $1;}
	| SHORT { $$ = $1;}
	| INT { $$ = $1;}
	| LONG { $$ = $1;}
	| SIGNED { $$ = $1;}
	| UNSIGNED { $$ = $1;}
	| FLOAT { $$ = $1;}
	| DOUBLE { $$ = $1;}
	| VOID { $$ = $1;}
	;

type_name:
	class_name { $$ = $1; }
	| enum_name { $$ = $1; }
	| typedef_name { $$ = $1; }
	;

elaborated_type_specifier:
	  class_key COLONCOLON nested_name_specifier identifier { $$ = alctree(elaborated_type_specifier, 4, $1, $2, $3, $4); }
	| class_key COLONCOLON identifier { $$ = alctree(elaborated_type_specifier, 3, $1, $2, $3); }
	| ENUM COLONCOLON nested_name_specifier identifier { $$ = alctree(elaborated_type_specifier, 4, $1, $2, $3, $4); }
	| ENUM COLONCOLON identifier { $$ = alctree(elaborated_type_specifier, 3, $1, $2, $3); }
	| ENUM nested_name_specifier identifier { $$ = alctree(elaborated_type_specifier, 3, $1, $2, $3); }
	| TYPENAME COLONCOLON_opt nested_name_specifier identifier { $$ = alctree(elaborated_type_specifier, 4, $1, $2, $3, $4); }
	| TYPENAME COLONCOLON_opt nested_name_specifier identifier '<' template_argument_list '>' { $$ = alctree(elaborated_type_specifier, 7, $1, $2, $3, $4, $5, $6, $7); }
	;

/*
enum_name:
	identifier
	;
*/

enum_specifier:
	ENUM identifier '{' enumerator_list_opt '}' { $$ = NULL; yyerror("C++ operation not supported"); }
	;

enumerator_list:
	enumerator_definition { $$ = $1; }
	| enumerator_list ',' enumerator_definition { $$ = NULL; yyerror("C++ operation not supported"); }
	;

enumerator_definition:
	enumerator { $$ = $1; }
	| enumerator '=' constant_expression { $$ = NULL; yyerror("C++ operation not supported"); }
	;

enumerator:
	identifier { $$ = $1; }
	;

/*
namespace_name:
	original_namespace_name
	| namespace_alias
	;

original_namespace_name:
	identifier
	;
*/

namespace_definition:
	named_namespace_definition { $$ = NULL; yyerror("C++ operation not supported"); }
	| unnamed_namespace_definition { $$ = NULL; yyerror("C++ operation not supported"); }
	;

named_namespace_definition:
	original_namespace_definition { $$ = NULL; yyerror("C++ operation not supported"); }
	| extension_namespace_definition { $$ = NULL; yyerror("C++ operation not supported"); }
	;

original_namespace_definition:
	NAMESPACE identifier '{' namespace_body '}' { $$ = NULL; yyerror("C++ operation not supported"); }
	;

extension_namespace_definition:
	NAMESPACE original_namespace_name '{' namespace_body '}' { $$ = NULL; yyerror("C++ operation not supported"); }
	;

unnamed_namespace_definition:
	NAMESPACE '{' namespace_body '}' { $$ = NULL; yyerror("C++ operation not supported"); }
	;

namespace_body:
	declaration_seq_opt { $$ = NULL; yyerror("C++ operation not supported"); }
	;

/*
namespace_alias:
	identifier
	;
*/

namespace_alias_definition:
	NAMESPACE identifier '=' qualified_namespace_specifier ';' { $$ = NULL; yyerror("C++ operation not supported"); }
	;

qualified_namespace_specifier:
	  COLONCOLON nested_name_specifier namespace_name { $$ = NULL; yyerror("C++ operation not supported"); }
	| COLONCOLON namespace_name { $$ = NULL; yyerror("C++ operation not supported"); }
	| nested_name_specifier namespace_name { $$ = NULL; yyerror("C++ operation not supported"); }
	| namespace_name { $$ = NULL; yyerror("C++ operation not supported"); }
	;

using_declaration:
	  USING TYPENAME COLONCOLON nested_name_specifier unqualified_id ';' { $$ = NULL; yyerror("C++ operation not supported"); }
	| USING TYPENAME nested_name_specifier unqualified_id ';' { $$ = NULL; yyerror("C++ operation not supported"); }
	| USING COLONCOLON nested_name_specifier unqualified_id ';' { $$ = NULL; yyerror("C++ operation not supported"); }
	| USING nested_name_specifier unqualified_id ';' { $$ = NULL; yyerror("C++ operation not supported"); }
	| USING COLONCOLON unqualified_id ';' { $$ = NULL; yyerror("C++ operation not supported"); }
	;

using_directive:
	USING NAMESPACE COLONCOLON nested_name_specifier namespace_name ';' { $$ = NULL; yyerror("C++ operation not supported"); }
	| USING NAMESPACE COLONCOLON namespace_name ';' { $$ = NULL; yyerror("C++ operation not supported"); }
	| USING NAMESPACE nested_name_specifier namespace_name ';' { $$ = NULL; yyerror("C++ operation not supported"); }
	| USING NAMESPACE namespace_name ';' { $$ = NULL; yyerror("C++ operation not supported"); }
	;

asm_definition:
	ASM '(' string_literal ')' ';' { $$ = NULL; yyerror("C++ operation not supported"); }
	;

linkage_specification:
	EXTERN string_literal '{' declaration_seq_opt '}' { $$ = NULL; yyerror("C++ operation not supported"); }
	| EXTERN string_literal declaration { $$ = NULL; yyerror("C++ operation not supported"); }
	;

/*----------------------------------------------------------------------
 * Declarators.
 *----------------------------------------------------------------------*/

init_declarator_list:
	init_declarator { $$ = $1; }
	| init_declarator_list ',' init_declarator { $$ = alctree(init_declarator_list, 3, $1, $2, $3); }
	;

init_declarator:
	declarator initializer_opt { $$ = alctree(init_declarator, 2, $1, $2); }
	;

declarator:
	direct_declarator { $$ = $1; }
	| ptr_operator declarator  { $$ = alctree(declarator, 2, $1, $2); }
	;

direct_declarator:
	  declarator_id { $$ = alctree(direct_declarator_1, 1, $1); }
	| direct_declarator '(' parameter_declaration_clause ')' cv_qualifier_seq exception_specification { $$ = alctree(direct_declarator, 6, $1, $2, $3, $4, $5, $6); }
	| direct_declarator '(' parameter_declaration_clause ')' cv_qualifier_seq { $$ = alctree(direct_declarator, 5, $1, $2, $3, $4, $5); }
	| direct_declarator '(' parameter_declaration_clause ')' exception_specification { $$ = alctree(direct_declarator, 5, $1, $2, $3, $4, $5); }
	| direct_declarator '(' parameter_declaration_clause ')' { $$ = alctree(direct_declarator_4, 4, $1, $2, $3, $4); }
	| CLASS_NAME '(' parameter_declaration_clause ')' { $$ = alctree(direct_declarator_5, 4, $1, $2, $3, $4); }
	| CLASS_NAME COLONCOLON declarator_id '(' parameter_declaration_clause ')' { $$ = alctree(direct_declarator_6, 6, $1, $2, $3, $4, $5, $6); }
	| CLASS_NAME COLONCOLON CLASS_NAME '(' parameter_declaration_clause ')' { $$ = alctree(direct_declarator_7, 6, $1, $2, $3, $4, $5, $6); }
	| direct_declarator '[' constant_expression_opt ']' { $$ = alctree(direct_declarator_8, 4, $1, $2, $3, $4); }
	| '(' declarator ')' { $$ = alctree(direct_declarator, 3, $1, $2, $3); }
	;

ptr_operator:
	'*' { $$ = $1; }
	| '*' cv_qualifier_seq { $$ = alctree(ptr_operator, 2, $1, $2); }
	| '&' { $$ = $1; }
	| nested_name_specifier '*' { $$ = alctree(ptr_operator, 2, $1, $2); }
	| nested_name_specifier '*' cv_qualifier_seq { $$ = alctree(ptr_operator, 3, $1, $2, $3); }
	| COLONCOLON nested_name_specifier '*' { $$ = alctree(ptr_operator, 3, $1, $2, $3); }
	| COLONCOLON nested_name_specifier '*' cv_qualifier_seq { $$ = alctree(ptr_operator, 4, $1, $2, $3, $4); }
	;

cv_qualifier_seq:
	cv_qualifier { $$ = $1; }
	| cv_qualifier cv_qualifier_seq { $$ = alctree(cv_qualifier_seq, 2, $1, $2); }
	;

cv_qualifier:
	CONST { $$ = $1;}
	| VOLATILE { $$ = NULL; }

declarator_id:
	  id_expression { $$ = $1; }
	| COLONCOLON id_expression { $$ = alctree(declarator_id, 2, $1, $2); }
	| COLONCOLON nested_name_specifier type_name { $$ = alctree(declarator_id, 3, $1, $2, $3); }
	| COLONCOLON type_name { $$ = alctree(declarator_id, 2, $1, $2); }
	;

type_id:
	type_specifier_seq abstract_declarator_opt { $$ = alctree(type_id, 2, $1, $2); }
	;

type_specifier_seq:
	type_specifier type_specifier_seq_opt { $$ = alctree(type_specifier_seq, 2, $1, $2); }
	;

abstract_declarator:
	ptr_operator abstract_declarator_opt { $$ = alctree(abstract_declarator, 2, $1, $2); }
	| direct_abstract_declarator { $$ = $1; }
	;

direct_abstract_declarator:
	  direct_abstract_declarator_opt '(' parameter_declaration_clause ')' cv_qualifier_seq exception_specification { $$ = alctree(direct_abstract_declarator, 6, $1, $2, $3, $4, $5, $6); }
	| direct_abstract_declarator_opt '(' parameter_declaration_clause ')' cv_qualifier_seq { $$ = alctree(direct_abstract_declarator, 5, $1, $2, $3, $4, $5); }
	| direct_abstract_declarator_opt '(' parameter_declaration_clause ')' exception_specification { $$ = alctree(direct_abstract_declarator, 5, $1, $2, $3, $4, $5); }
	| direct_abstract_declarator_opt '(' parameter_declaration_clause ')' { $$ = alctree(direct_abstract_declarator, 4, $1, $2, $3, $4); }
	| direct_abstract_declarator_opt '[' constant_expression_opt ']' { $$ = alctree(direct_abstract_declarator, 4, $1, $2, $3, $4); }
	| '(' abstract_declarator ')' { $$ = alctree(direct_abstract_declarator, 3, $1, $2, $3); }
	;

parameter_declaration_clause:
	  parameter_declaration_list ELLIPSIS { $$ = alctree(parameter_declaration_clause, 2, $1, $2); }
	| parameter_declaration_list { $$ = $1; }
	| ELLIPSIS { $$ = NULL; }
	| { $$ = NULL; }
	| parameter_declaration_list ',' ELLIPSIS { $$ = alctree(parameter_declaration_clause, 3, $1, $2, $3); }
	;

parameter_declaration_list:
	parameter_declaration { $$ = $1; }
	| parameter_declaration_list ',' parameter_declaration { $$ = alctree(parameter_declaration_list, 3, $1, $2, $3); }
	;

parameter_declaration:
	decl_specifier_seq declarator { $$ = alctree(parameter_declaration, 2, $1, $2); }
	| decl_specifier_seq declarator '=' assignment_expression { $$ = alctree(parameter_declaration, 4, $1, $2, $3, $4); }
	| decl_specifier_seq abstract_declarator_opt { $$ = alctree(parameter_declaration, 2, $1, $2); }
	| decl_specifier_seq abstract_declarator_opt '=' assignment_expression { $$ = alctree(parameter_declaration, 4, $1, $2, $3, $4); }
	;

function_definition:
	  declarator ctor_initializer_opt function_body { $$ = alctree(ctor_function_definition, 3, $1, $2, $3); }
	| decl_specifier_seq declarator ctor_initializer_opt function_body { $$ = alctree(function_definition_1, 4, $1, $2, $3, $4); }
	| declarator function_try_block { $$ = alctree(function_definition, 2, $1, $2); }
	| decl_specifier_seq declarator function_try_block { $$ = alctree(function_definition, 3, $1, $2, $3); }
	;

function_body:
	compound_statement { $$ = $1; }
	;

initializer:
	'=' initializer_clause { $$ = alctree(initializer, 1, $2); }
	| '(' expression_list ')' { $$ = alctree(initializer, 3, $1, $2, $3); }
	;

initializer_clause:
	assignment_expression { $$ = $1; }
	| '{' initializer_list COMMA_opt '}' { $$ = alctree(initializer_clause, 4, $1, $2, $3, $4); }
	| '{' '}' { $$ = alctree(initializer_clause, 2, $1, $2); }
	;

initializer_list:
	initializer_clause { $$ = $1; }
	| initializer_list ',' initializer_clause { $$ = alctree(initializer_list, 3, $1, $2, $3); }
	;

/*----------------------------------------------------------------------
 * Classes.
 *----------------------------------------------------------------------*/

class_specifier:
	class_head '{' member_specification_opt '}' { $$ = alctree(class_specifier, 4, $1, $2, $3, $4); }
	;

class_head:
	  class_key identifier { typenametable_insert($2->leaf->text, CLASS_NAME);
				$$ = alctree(class_head, 2, $1, $2); } 
	| class_key identifier base_clause { $$ = alctree(class_head, 3, $1, $2, $3); }
	| class_key nested_name_specifier identifier { $$ = alctree(class_head, 3, $1, $2, $3); }
	| class_key nested_name_specifier identifier base_clause { $$ = alctree(class_head, 4, $1, $2, $3, $4); }
	;

class_key:
	CLASS { $$ = $1;}
	| STRUCT { $$ = $1;}
	| UNION { $$ = NULL; }
	;

member_specification:
	member_declaration member_specification_opt { $$ = alctree(member_specification, 2, $1, $2); }
	| access_specifier ':' member_specification_opt { $$ = alctree(member_specification_2, 3, $1, $2, $3); }
	;

member_declaration:
	  decl_specifier_seq member_declarator_list ';' { $$ = alctree(member_declaration_1, 3, $1, $2, $3); }
	| decl_specifier_seq ';' { $$ = alctree(member_declaration, 2, $1, $2); }
	| member_declarator_list ';' { $$ = alctree(member_declaration_3, 2, $1, $2); }
	| ';' { $$ = $1;}
	| function_definition SEMICOLON_opt { $$ = alctree(member_declaration_5, 2, $1, $2); }
	| qualified_id ';' { $$ = alctree(member_declaration_6, 2, $1, $2); }
	| using_declaration { $$ = NULL; yyerror("C++ operation not supported"); }
	| template_declaration { $$ = NULL; yyerror("C++ operation not supported"); }
	;

member_declarator_list:
	member_declarator { $$ = alctree(member_declarator_list_1, 1, $1); }
	| member_declarator_list ',' member_declarator { $$ = alctree(member_declarator_list, 3, $1, $2, $3); }
	;

member_declarator:
	  declarator { $$ = $1; }
	| declarator pure_specifier { $$ = alctree(member_declarator, 2, $1, $2); }
	| declarator constant_initializer { $$ = alctree(member_declarator, 2, $1, $2); }
	| identifier ':' constant_expression { $$ = alctree(member_declarator, 3, $1, $2, $3); }
	;

/*
 * This rule need a hack for working around the ``= 0'' pure specifier.
 * 0 is returned as an ``INTEGER'' by the lexical analyzer but in this
 * context is different.
 */
pure_specifier:
	'=' '0' { $$ = alctree(pure_specifier, 2, $1, $2); }
	;

constant_initializer:
	'=' constant_expression { $$ = alctree(constant_initializer, 2, $1, $2); }
	;

/*----------------------------------------------------------------------
 * Derived classes.
 *----------------------------------------------------------------------*/

base_clause:
	':' base_specifier_list { $$ = NULL; yyerror("C++ operation not supported"); }
	;

base_specifier_list:
	base_specifier { $$ = NULL; yyerror("C++ operation not supported"); }
	| base_specifier_list ',' base_specifier { $$ = NULL; yyerror("C++ operation not supported"); }
	;

base_specifier:
	  COLONCOLON nested_name_specifier class_name { $$ = NULL; yyerror("C++ operation not supported"); }
	| COLONCOLON class_name { $$ = alctree(base_specifier, 2, $1, $2); }
	| nested_name_specifier class_name { $$ = NULL; yyerror("C++ operation not supported"); }
	| class_name { $$ = NULL; yyerror("C++ operation not supported"); }
	| VIRTUAL access_specifier COLONCOLON nested_name_specifier_opt class_name { $$ = NULL; yyerror("C++ operation not supported"); }
	| VIRTUAL access_specifier nested_name_specifier_opt class_name { $$ = NULL; yyerror("C++ operation not supported"); }
	| VIRTUAL COLONCOLON nested_name_specifier_opt class_name { $$ = NULL; yyerror("C++ operation not supported"); }
	| VIRTUAL nested_name_specifier_opt class_name { $$ = NULL; yyerror("C++ operation not supported"); }
	| access_specifier VIRTUAL COLONCOLON nested_name_specifier_opt class_name { $$ = alctree(base_specifier, 5, $1, $2, $3, $4, $5); }
	| access_specifier VIRTUAL nested_name_specifier_opt class_name { $$ = NULL; yyerror("C++ operation not supported"); }
	| access_specifier COLONCOLON nested_name_specifier_opt class_name { $$ = NULL; yyerror("C++ operation not supported"); }
	| access_specifier nested_name_specifier_opt class_name { $$ = NULL; yyerror("C++ operation not supported"); }
	;

access_specifier:
	PRIVATE { $$ = $1;}
	| PROTECTED { $$ = NULL;}
	| PUBLIC { $$ = $1;}
	;

/*----------------------------------------------------------------------
 * Special member functions.
 *----------------------------------------------------------------------*/

conversion_function_id:
	OPERATOR conversion_type_id { $$ = NULL; yyerror("C++ operation not supported"); }
	;

conversion_type_id:
	type_specifier_seq conversion_declarator_opt { $$ = NULL; yyerror("C++ operation not supported"); }
	;

conversion_declarator:
	ptr_operator conversion_declarator_opt { $$ = NULL; yyerror("C++ operation not supported"); }
	;

ctor_initializer:
	':' mem_initializer_list { $$ = NULL; yyerror("C++ operation not supported"); }
	;

mem_initializer_list:
	mem_initializer { $$ = NULL; yyerror("C++ operation not supported"); }
	| mem_initializer ',' mem_initializer_list { $$ = NULL; yyerror("C++ operation not supported"); }
	;

mem_initializer:
	mem_initializer_id '(' expression_list_opt ')' { $$ = NULL; yyerror("C++ operation not supported"); }
	;

mem_initializer_id:
	  COLONCOLON nested_name_specifier class_name { $$ = NULL; yyerror("C++ operation not supported"); }
	| COLONCOLON class_name { $$ = NULL; yyerror("C++ operation not supported"); }
	| nested_name_specifier class_name { $$ = NULL; yyerror("C++ operation not supported"); }
	| class_name { $$ = NULL; yyerror("C++ operation not supported"); }
	| identifier { $$ = NULL; yyerror("C++ operation not supported"); }
	;

/*----------------------------------------------------------------------
 * Overloading.
 *----------------------------------------------------------------------*/

operator_function_id:
	OPERATOR operator { $$ = NULL; yyerror("C++ operation not supported"); }
	;

operator:
	NEW { $$ = $1;}
	| DELETE { $$ = $1;}
	| NEW '[' ']' { $$ = alctree(operator, 3, $1, $2, $3); }
	| DELETE '[' ']' { $$ = alctree(operator, 3, $1, $2, $3); }
	| '+' { $$ = $1;}
	| '_' { $$ = $1;}
	| '*' { $$ = $1;}
	| '/' { $$ = $1;}
	| '%' { $$ = $1;}
	| '^' { $$ = $1;}
	| '&' { $$ = $1;}
	| '|' { $$ = $1;}
	| '~' { $$ = $1;}
	| '!' { $$ = $1;}
	| '=' { $$ = $1;}
	| '<' { $$ = $1;}
	| '>' { $$ = $1;}
	| ADDEQ { $$ = $1;}
	| SUBEQ { $$ = $1;}
	| MULEQ { $$ = $1;}
	| DIVEQ { $$ = $1;}
	| MODEQ { $$ = $1;}
	| XOREQ { $$ = $1;}
	| ANDEQ { $$ = $1;}
	| OREQ { $$ = $1;}
	| SL { $$ = $1;} 
	| SR { $$ = $1;}
	| SREQ { $$ = NULL;}
	| SLEQ { $$ = NULL;}
	| EQ { $$ = $1;}
	| NOTEQ { $$ = $1;}
	| LTEQ { $$ = $1;}
	| GTEQ { $$ = $1;}
	| ANDAND { $$ = $1;}
	| OROR { $$ = $1;}
	| PLUSPLUS { $$ = $1;}
	| MINUSMINUS { $$ = $1;}
	| ',' { $$ = $1;}
	| ARROWSTAR { $$ = $1; }
	| ARROW { $$ = $1;}
	| '(' ')' { $$ = alctree(operator, 2, $1, $2); }
	| '[' ']' { $$ = alctree(operator, 2, $1, $2); }
	;

/*----------------------------------------------------------------------
 * Templates.
 *----------------------------------------------------------------------*/

template_declaration:
	EXPORT_opt TEMPLATE '<' template_parameter_list '>' declaration { $$ = NULL; yyerror("C++ operation not supported"); } 
	;

template_parameter_list:
	template_parameter { $$ = NULL; yyerror("C++ operation not supported"); }
	| template_parameter_list ',' template_parameter { $$ = NULL; yyerror("C++ operation not supported"); }
	;

template_parameter:
	type_parameter { $$ = NULL; yyerror("C++ operation not supported"); }
	| parameter_declaration { $$ = NULL; yyerror("C++ operation not supported"); }
	;

type_parameter:
	  CLASS identifier { $$ = NULL; yyerror("C++ operation not supported"); }
	| CLASS identifier '=' type_id { $$ = NULL; yyerror("C++ operation not supported"); }
	| TYPENAME identifier { $$ = NULL; yyerror("C++ operation not supported"); }
	| TYPENAME identifier '=' type_id { $$ = NULL; yyerror("C++ operation not supported"); }
	| TEMPLATE '<' template_parameter_list '>' CLASS identifier { $$ = NULL; yyerror("C++ operation not supported"); }
	| TEMPLATE '<' template_parameter_list '>' CLASS identifier '=' template_name { $$ = NULL; yyerror("C++ operation not supported"); }
	;

template_id:
	template_name '<' template_argument_list '>' { $$ = NULL; yyerror("C++ operation not supported"); }
	;

template_argument_list:
	template_argument { $$ = NULL; yyerror("C++ operation not supported"); }
	| template_argument_list ',' template_argument { $$ = NULL; yyerror("C++ operation not supported"); }
	;

template_argument:
	assignment_expression { $$ = NULL; yyerror("C++ operation not supported"); }
	| type_id { $$ = NULL; yyerror("C++ operation not supported"); }
	| template_name { $$ = NULL; yyerror("C++ operation not supported"); }
	;

explicit_instantiation:
	TEMPLATE declaration { $$ = NULL; yyerror("C++ operation not supported"); }
	;

explicit_specialization:
	TEMPLATE '<' '>' declaration { $$ = NULL; yyerror("C++ operation not supported"); }
	;

/*----------------------------------------------------------------------
 * Exception handling.
 *----------------------------------------------------------------------*/

try_block:
	TRY compound_statement handler_seq { $$ = NULL; yyerror("C++ operation not supported"); }
	;

function_try_block:
	TRY ctor_initializer_opt function_body handler_seq { $$ = NULL; yyerror("C++ operation not supported"); }
	;

handler_seq:
	handler handler_seq_opt { $$ = NULL; yyerror("C++ operation not supported"); }
	;

handler:
	CATCH '(' exception_declaration ')' compound_statement { $$ = NULL; yyerror("C++ operation not supported"); }
	;

exception_declaration:
	type_specifier_seq declarator { $$ = NULL; yyerror("C++ operation not supported"); }
	| type_specifier_seq abstract_declarator { $$ = NULL; yyerror("C++ operation not supported"); }
	| type_specifier_seq { $$ = NULL; yyerror("C++ operation not supported"); }
	| ELLIPSIS { $$ = NULL;}
	;

throw_expression:
	THROW assignment_expression_opt { $$ = NULL; yyerror("C++ operation not supported"); }
	;

exception_specification:
	THROW '(' type_id_list_opt ')' { $$ = NULL; yyerror("C++ operation not supported"); }
	;

type_id_list:
	type_id { $$ = NULL; yyerror("C++ operation not supported"); }
	| type_id_list ',' type_id { $$ = NULL; yyerror("C++ operation not supported"); }
	;

/*----------------------------------------------------------------------
 * Epsilon (optional) definitions.
 *----------------------------------------------------------------------*/

declaration_seq_opt:
	/* epsilon */ { $$ = NULL; }
	| declaration_seq { $$ = $1; }
	;

nested_name_specifier_opt:
	/* epsilon */ { $$ = NULL; }
	| nested_name_specifier { $$ = $1; }
	;

expression_list_opt:
	/* epsilon */ { $$ = NULL; }
	| expression_list { $$ = $1; }
	;

COLONCOLON_opt:
	/* epsilon */ { $$ = NULL; }
	| COLONCOLON { $$ = $1;}
	;

new_placement_opt:
	/* epsilon */ { $$ = NULL; }
	| new_placement { $$ = $1; }
	;

new_initializer_opt:
	/* epsilon */ { $$ = NULL; }
	| new_initializer { $$ = $1; }
	;

new_declarator_opt:
	/* epsilon */ { $$ = NULL; }
	| new_declarator { $$ = $1; }
	;

expression_opt:
	/* epsilon */ { $$ = NULL; }
	| expression { $$ = $1; }
	;

statement_seq_opt:
	/* epsilon */ { $$ = NULL; }
	| statement_seq { $$ = $1; }
	;

condition_opt:
	/* epsilon */ { $$ = NULL; }
	| condition { $$ = $1; }
	;

enumerator_list_opt:
	/* epsilon */ { $$ = NULL; }
	| enumerator_list { $$ = $1; }
	;

initializer_opt: 
	/* epsilon */ { $$ = NULL; }
	| initializer { $$ = $1; }
	;

constant_expression_opt:
	/* epsilon */ { $$ = NULL; }
	| constant_expression { $$ = $1; }
	;

abstract_declarator_opt:
	/* epsilon */ { $$ = NULL; }
	| abstract_declarator { $$ = $1; }
	;

type_specifier_seq_opt:
	/* epsilon */ { $$ = NULL; }
	| type_specifier_seq { $$ = $1; }
	;

direct_abstract_declarator_opt:
	/* epsilon */  { $$ = NULL; }
	| direct_abstract_declarator { $$ = $1; }
	;

ctor_initializer_opt:
	/* epsilon */ { $$ = NULL; }
	| ctor_initializer { $$ = $1; }
	;

COMMA_opt:
	/* epsilon */ { $$ = NULL; }
	| ',' { $$ = $1;}
	;

member_specification_opt:
	/* epsilon */ { $$ = NULL; }
	| member_specification { $$ = $1; }
	;

SEMICOLON_opt:
	/* epsilon */ { $$ = NULL; }
	| ';' { $$ = $1;}
	;

conversion_declarator_opt:
	/* epsilon */ { $$ = NULL; }
	| conversion_declarator { $$ = $1; }
	;

EXPORT_opt:
	/* epsilon */ { $$ = NULL; }
	| EXPORT { $$ = $1;}
	;

handler_seq_opt:
	/* epsilon */ { $$ = NULL; }
	| handler_seq { $$ = $1; }
	;

assignment_expression_opt:
	/* epsilon */ { $$ = NULL; }
	| assignment_expression { $$ = $1; }
	;

type_id_list_opt:
	/* epsilon */ { $$ = NULL; }
	| type_id_list { $$ = $1; }
	;

%%

static void
yyerror(char *s)
{
	fprintf(stderr, "%s:%d: %s before '%s' token\n",
	   filename, lineno, s, yytext);
	exit(2);
}




#include "tree.h"
#include "120gram.h"

extern int lineno;
extern char* filename;

struct tree *alctree(int label, int nkids, ...)
{
   int i;
   va_list ap;
   struct tree *ptr = malloc(sizeof(struct tree) +
                             (nkids-1)*sizeof(struct tree *));
   if (ptr == NULL) {fprintf(stderr, "alctree out of memory\n"); exit(1); }
   ptr->label = label;
   ptr->nkids = nkids;
   ptr->code = NULL;
   ptr->leaf = NULL;
   ptr->place.offset = 0;
   ptr->place.region = UNKNOWN_R;
   ptr->place.type = NULL;
   va_start(ap, nkids);
   for(i=0; i < nkids; i++) {
      ptr->kids[i] = va_arg(ap, struct tree *);
   }
   return ptr;
}

struct tree *alcnode(struct token *t)
{
   struct tree *node = (struct tree *)malloc(sizeof(struct tree));
   node->leaf = t;
   node->code = NULL;
   node->label = 0;
   node->place.region = UNKNOWN_R;
   node->place.offset = 0;
   node->place.type = NULL;
   node->nkids = -1; /*nkids set to -1 so I'm able to tell difference 
			between terminals and nonterminals */ 
   return node; 
}
	
void printer(struct tree *t, int depth)
{
   if (t == NULL)
      return;
   else if (t->nkids == -1)
      printf("%*s %s  %d\n", depth*2, " ", t->leaf->text, t->leaf->category);
   //Not really needed but added so program child was child instead of children
   else if (t->nkids == 1)
      printf("%*s%s %d: %d child\n", depth*2, "", humanreadable(t->label), t->label, t->nkids);
   else
      printf("%*s%s %d: %d children\n", depth*2, "", humanreadable(t->label), t->label, t->nkids);

}

void preorder(struct tree *t, int depth)
{
   int i;
   if (t == NULL) 
    {
      printer (t, depth);
      return;
     }
   printer(t, depth);
   for (i=0; i < t-> nkids; i++)
      preorder(t->kids[i], depth+1);

}

void semantic_error(char *s, struct tree *t)
{
	while(1){
		if(t->nkids == -1)
			break;
		else
			t = t->kids[0];
		
	}
  	fprintf(stderr, "%s:%d: %s '%s'\n",
		   t->leaf->filename, t->leaf->lineno, s, t->leaf->text);
  	exit(3);
}

struct tree *getNode(struct tree *t, int child){
	if(t->nkids > 0)
		t = t->kids[child];
	if(t == NULL)
		return NULL;
	return t;
}

char* humanreadable(int rule) {

char* rulestring; 

switch (rule) {
     case typedef_name : 
	rulestring = "typedef_name";
	break;
case namespace_name : 
	rulestring = "namespace_name";
	break;
case original_namespace_name : 
	rulestring = "original_namespace_name";
	break;
case class_name : 
	rulestring = "class_name";
	break;
case enum_name : 
	rulestring = "enum_name";
	break;
case template_name : 
	rulestring = "template_name";
	break;
case identifier : 
	rulestring = "identifier";
	break;
case literal : 
	rulestring = "literal";
	break;
case integer_literal :   
	rulestring = "integer_literal";
	break;
case character_literal : 
	rulestring = "character_literal";
	break;
case floating_literal : 
	rulestring = "floating_literal";
	break;
case string_literal : 
	rulestring = "string_literal";
	break;
case boolean_literal : 
	rulestring = "boolean_literal";
	break;
case program : 
	rulestring = "program";
	break;
case primary_expression : 
	rulestring = "primary_expression";
	break;
case id_expression :
	rulestring = "id_expression";
	break;
case unqualified_id :
	rulestring = "unqualified_id";
	break;
case qualified_id :
	rulestring = "qualified_id";
	break;
case nested_name_specifier :
	rulestring = "nested_name_specifier";
	break;
case postfix_expression :
	rulestring = "postfix_expression";
	break;
case expression_list :
	rulestring = "expression_list";
	break;
case unary_expression :
	rulestring = "unary_expression";
	break;
case unary_operator :
	rulestring = "unary_operator";
	break;
case new_expression :
	rulestring = "new_expression";
	break;
case new_placement :
	rulestring = "new_placement";
	break;
case new_type_id :
	rulestring = "new_type_id";
	break;
case new_declarator :
	rulestring = "new_declarator";
	break;
case direct_new_declarator :
	rulestring = "direct_new_declarator";
	break;
case new_initializer :
	rulestring = "new_initializer";
	break;
case delete_expression :
	rulestring = "delete_expression";
	break;
case cast_expression :
	rulestring = "cast_expression";
	break;
case pm_expression :
	rulestring = "pm_expression";
	break;
case multiplicative_expression :
	rulestring = "multiplicative_expression";
	break;
case additive_expression :
	rulestring = "additive_expression";
	break;
case shift_expression :
	rulestring = "shift_expression";
	break;
case relational_expression :
	rulestring = "relational_expression";
	break;
case equality_expression :
	rulestring = "equality_expression";
	break;
case and_expression :
	rulestring = "and_expression";
	break;
case exclusive_or_expression :
	rulestring = "exclusive_or_expression";
	break;
case inclusive_or_expression :
	rulestring = "inclusive_or_expression";
	break;
case logical_and_expression :
	rulestring = "logical_and_expression";
	break;
case logical_or_expression :
	rulestring = "logical_or_expression";
	break;
case conditional_expression :
	rulestring = "conditional_expression";
	break;
case assignment_expression :
	rulestring = "assignment_expression";
	break;
case assignment_operator :
	rulestring = "assignment_operator";
	break;
case expression :
	rulestring = "expression";
	break;
case constant_expression :
	rulestring = "constant_expression";
	break;
case statement :
	rulestring = "statement";
	break;
case labeled_statement :
	rulestring = "labeled_statement";
	break;
case expression_statement :
	rulestring = "expression_statement";
	break;
case compound_statement :
	rulestring = "compound_statement";
	break;
case statement_seq :
	rulestring = "statement_seq";
	break;
case selection_statement :
	rulestring = "selection_statement";
	break;
case condition :
	rulestring = "condition";
	break;
case iteration_statement :
	rulestring = "iteration_statement";
	break;
case for_init_statement :
	rulestring = "for_init_statement";
	break;
case jump_statement :
	rulestring = "jump_statement";
	break;
case declaration_statement :
	rulestring = "declaration_statement";
	break;
case declaration_seq :
	rulestring = "declaration_seq";
	break;
case declaration :
	rulestring = "declaration";
	break;
case block_declaration :
	rulestring = "block_declaration";
	break;
case simple_declaration :
	rulestring = "simple_declaration";
	break;
case decl_specifier :
	rulestring = "decl_specifier";
	break;
case decl_specifier_seq :
	rulestring = "decl_specifier_seq";
	break;
case storage_class_specifier :
	rulestring = "storage_class_specifier";
	break;
case function_specifier :
	rulestring = "function_specifier";
	break;
case type_specifier :
	rulestring = "type_specifier";
	break;
case simple_type_specifier :
	rulestring = "simple_type_specifier";
	break;
case type_name :
	rulestring = "type_name";
	break;
case elaborated_type_specifier :
	rulestring = "elaborated_type_specifier";
	break;
case enum_specifier :
	rulestring = "enum_specifier";
	break;
case enumerator_list :
	rulestring = "enumerator_list";
	break;
case enumerator_definition :
	rulestring = "enumerator_definition";
	break;
case enumerator :
	rulestring = "enumerator";
	break;
case namespace_definition :
	rulestring = "namespace_definition";
	break;
case named_namespace_definition :
	rulestring = "named_namespace_definition";
	break;
case original_namespace_definition :
	rulestring = "original_namespace_definition";
	break;
case extension_namespace_definition :
	rulestring = "extension_namespace_definition";
	break;
case unnamed_namespace_definition :
	rulestring = "unnamed_namespace_definition";
	break;
case namespace_body :
	rulestring = "namespace_body";
	break;
case namespace_alias_definition :
	rulestring = "namespace_alias_definition";
	break;
case qualified_namespace_specifier :
	rulestring = "qualified_namespace_specifier";
	break;
case using_declaration :
	rulestring = "using_declaration";
	break;
case using_directive :
	rulestring = "using_directive";
	break;
case asm_definition :
	rulestring = "asm_definition";
	break;
case linkage_specification :
	rulestring = "linkage_specification";
	break;
case init_declarator_list :
	rulestring = "init_declarator_list";
	break;
case init_declarator :
	rulestring = "init_declarator";
	break;
case declarator :
	rulestring = "declarator";
	break;
case direct_declarator :
	rulestring = "direct_declarator";
	break;
case ptr_operator :
	rulestring = "ptr_operator";
	break;
case cv_qualifier_seq :
	rulestring = "cv_qualifier_seq";
	break;
case cv_qualifier :
	rulestring = "cv_qualifier";
	break;
case declarator_id :
	rulestring = "declarator_id";
	break;
case type_id :
	rulestring = "type_id";
	break;
case type_specifier_seq :
	rulestring = "type_specifier_seq";
	break;
case abstract_declarator :
	rulestring = "abstract_declarator";
	break;
case direct_abstract_declarator :
	rulestring = "direct_abstract_declarator";
	break;
case parameter_declaration_clause :
	rulestring = "parameter_declaration_clause";
	break;
case parameter_declaration_list :
	rulestring = "parameter_declaration_list";
	break;
case parameter_declaration :
	rulestring = "parameter_declaration";
	break;
case function_definition :
	rulestring = "function_definition";
	break;
case function_body :
	rulestring = "function_body";
	break;
case initializer :
	rulestring = "initializer";
	break;
case initializer_clause :
	rulestring = "initializer_clause";
	break;
case initializer_list :
	rulestring = "initializer_list";
	break;
case class_specifier :
	rulestring = "class_specifier";
	break;
case class_head :
	rulestring = "class_head";
	break;
case class_key :
	rulestring = "class_key";
	break;
case member_specification :
	rulestring = "member_specification";
	break;
case member_declaration :
	rulestring = "member_declaration";
	break;
case member_declarator_list :
	rulestring = "member_declarator_list";
	break;
case member_declarator :
	rulestring = "member_declarator";
	break;
case pure_specifier :
	rulestring = "pure_specifier";
	break;
case constant_initializer :
	rulestring = "constant_initializer";
	break;
case base_clause :
	rulestring = "base_clause";
	break;
case base_specifier_list :
	rulestring = "base_specifier_list";
	break;
case base_specifier :
	rulestring = "base_specifier";
	break;
case access_specifier :
	rulestring = "access_specifier";
	break;
case conversion_function_id :
	rulestring = "conversion_function_id";
	break;
case conversion_type_id :
	rulestring = "conversion_type_id";
	break;
case conversion_declarator :
	rulestring = "conversion_declarator";
	break;
case ctor_initializer :
	rulestring = "ctor_initializer";
	break;
case mem_initializer_list :
	rulestring = "mem_initializer_list";
	break;
case mem_initializer :
	rulestring = "mem_initializer";
	break;
case mem_initializer_id :
	rulestring = "mem_initializer_id";
	break;
case operator_function_id :
	rulestring = "operator_function_id";
	break;
case operator :
	rulestring = "operator";
	break;
case template_declaration :
	rulestring = "template_declaration";
	break;
case template_parameter_list :
	rulestring = "template_parameter_list";
	break;
case template_parameter :
	rulestring = "template_parameter";
	break;
case type_parameter :
	rulestring = "type_parameter";
	break;
case template_id :
	rulestring = "template_id";
	break;
case template_argument_list :
	rulestring = "template_argument_list";
	break;
case template_argument :
	rulestring = "template_argument";
	break;
case explicit_instantiation :
	rulestring = "explicit_instantiation";
	break;
case explicit_specialization :
	rulestring = "explicit_specialization";
	break;
case try_block :
	rulestring = "try_block";
	break;
case function_try_block :
	rulestring = "function_try_block";
	break;
case handler_seq :
	rulestring = "handler_seq";
	break;
case handler :
	rulestring = "handler";
	break;
case exception_declaration :
	rulestring = "exception_declaration";
	break;
case throw_expression :
	rulestring = "throw_expression";
	break;
case exception_specification :
	rulestring = "exception_specification";
	break;
case type_id_list :
	rulestring = "type_id_list";
	break;
case declaration_seq_opt :
	rulestring = "declaration_seq_opt";
	break;
case nested_name_specifier_opt :
	rulestring = "nested_name_specifier_opt";
	break;
case expression_list_opt :
	rulestring = "expression_list_opt";
	break;
case COLONCOLON_opt :
	rulestring = "COLONCOLON_opt";
	break;
case new_placement_opt :
	rulestring = "new_placement_opt";
	break;
case new_initializer_opt :
	rulestring = "new_initializer_opt";
	break;
case new_declarator_opt :
	rulestring = "new_declarator_opt";
	break;
case expression_opt :
	rulestring = "expression_opt";
	break;
case statement_seq_opt :
	rulestring = "statement_seq_opt";
	break;
case condition_opt :
	rulestring = "condition_opt";
	break;
case enumerator_list_opt :
	rulestring = "enumerator_list_opt";
	break;
case initializer_opt :
	rulestring = "initializer_opt";
	break;
case constant_expression_opt :
	rulestring = "constant_expression_opt";
	break;
case abstract_declarator_opt :
	rulestring = "abstract_declarator_opt";
	break;
case type_specifier_seq_opt :
	rulestring = "type_specifier_seq_opt";
	break;
case direct_abstract_declarator_opt :
	rulestring = "direct_abstract_declarator_opt";
	break;
case ctor_initializer_opt :
	rulestring = "ctor_initializer_opt";
	break;
case COMMA_opt :
	rulestring = "COMMA_opt";
	break;
case member_specification_opt :
	rulestring = "member_specification_opt";
	break;
case SEMICOLON_opt :
	rulestring = "SEMICOLON_opt";
	break;
case conversion_declarator_opt :
	rulestring = "conversion_declarator_opt";
	break;
case EXPORT_opt :
	rulestring = "EXPORT_opt";
	break;
case handler_seq_opt :
	rulestring = "handler_seq_opt";
	break;
case assignment_expression_opt :
	rulestring = "assignment_expression_opt";
	break;
case type_id_list_opt :
	rulestring = "type_id_list_opt";
	break;
case function_definition_1:
	rulestring = "function_definition_1";
	break;
case iteration_statement_3:
	rulestring = "iteration_statement_3";
	break;
case simple_declaration_1:
	rulestring = "simple_declaration_1";
	break;
case member_declaration_1:
	rulestring = "member_declaration_1";
	break;
case member_declarator_list_1:
	rulestring = "member_declarator_list_1";
	break;
case member_declarator_list_5:
	rulestring = "member_declarator_list_5";
	break;
case direct_declarator_4:
	rulestring = "direct_declarator_4";
	break;
case direct_declarator_5:
	rulestring = "direct_declarator_5";
	break;
case member_declaration_5:
	rulestring = "member_declaration_5";
	break;
case direct_declarator_1:
	rulestring = "direct_declarator_1";
	break;
case direct_declarator_8:
	rulestring = "direct_declarator_8";
	break;
case member_specification_2:
	rulestring = "member_specification_2";
	break;
case primary_expression_1:
	rulestring = "primary_expression_1";
	break;
case direct_declarator_6:
	rulestring = "direct_declarator_6";
	break;
case direct_declarator_7:
	rulestring = "direct_declarator_6";
	break;
case postfix_expression_1:
	rulestring = "postfix_expression_1";
	break;
case postfix_expression_2:
	rulestring = "postfix_expression_2";
	break;
case primary_expression_4:
	rulestring = "primary_expression_4";
	break;
case modulus_expression:
	rulestring = "modulus_expression";
	break;
case postfix_dot_expression:
	rulestring = "postfix_dot_expression";
	break;
case postfix_arrow_expression:
	rulestring = "postfix_arrow_expression";
	break;
case unary_op:
	rulestring = "unary_op";
	break;
case ctor_function_definition:
	rulestring = "ctor_function_definition";
	break;
case unary_sizeof:
	rulestring = "unary_sizeof";
	break;
case unary_star:
	rulestring = "unary_star";
	break;
case unary_address:
	rulestring = "unary_address";
	break;
case shift_left:
	rulestring = "shift_left";
	break;
case shift_right:
	rulestring = "shift_right";
	break;
case return_statement:
	rulestring = "return_statement";
	break;
case member_declaration_3:
	rulestring = "member_declaration_3";
	break;
case member_declaration_6:
	rulestring = "member_declaration_6";
	break;
case sub_expression:
	return "sub_expression";
case div_expression:
	return "div_expression";
case relational_lt:
	return "relational_lt";
case relational_gt:
	return "relational_gt";
case relational_lteq:
	return "relational_lteq";
case relational_gteq:
	return "relational_gteq";
case equality_eq:
	return "equality_eq";
case equality_neq:
	return "equality_neq";
case unary_plusplus:
	return "unary_plusplus";
case unary_minusminus:
	return "unary_minusminus";
case default_statement:
	return "default_statement";
case if_statement:
	return "if_statment";
case ifelse_statement:
	return "ifelse_statement";
case switch_statement:
	return "switch_statement";
case while_statement:
	return "while_statement";
case do_statement:
	return "do_statement";
case for_statement:
	return "for_statement";
case break_statement:
	return "break_statement";
case continue_statement:
	return "continue_statement";
case goto_statement:
	return "goto_statement";
case postfix_plusplus:
	return "postfix_plusplus";
case postfix_minusminus:
	return "postfix_minusminus";
default:
       	printf("Unrecognized production rule.\n");
    }
    return rulestring;
}

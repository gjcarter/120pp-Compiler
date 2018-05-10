/*	$Id: lexer.l,v 1.2 1997/11/19 15:13:15 sandro Exp $	*/

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
 * ISO C++ lexical analyzer.
 *
 * Based on the ISO C++ draft standard of December '96.
 */


O        [0-7]
D        [0-9]
L        [a-zA-Z]
H        [a-fA-F0-9]
E        [Ee][+-]?{D}+
FS       (f|F|l|L)
IS       (u|U|l|L)
W        [ \t\f]*
LIT      \"(\\.|[^\\"])*\"
US	 "_"
%option nounput
%option header-file="120lex.h"

%{
#include <ctype.h>
#include <stdio.h>
#include <stdbool.h>
#include "120gram.h"
#include "symtable.h"
#include "token.h"
#include "tree.h"
#include "type.h"

extern struct typeinfo class_t;

int lineno = 1;
bool saw_iostream, saw_string, saw_std, saw_fstream, saw_cstdlib, saw_ctime, saw_cmath = false;
bool include_io, include_string, include_fstream = false;

//Handles include file opening
void handle_include(); 
//Initializes token struct and fills it with data
int token(int intCode); 
//Removes quotes from sval and fixes escape chars
void fix_sval(char* sval); 
 //Pops file data from stack
void pop_file();
//Pushes file data onto stack 
void push_file(char *filename, int lineno); 

static int yywrap(void);
static void skip_until_eol(void);
static void skip_comment(void);
static int check_identifier(char *);
void freetypenametable();
int typenametable_insert(char *s, int cat);
int typenametable_lookup(char *s);
void lexerr(char *s);
void handle_string();
void handle_fstream();
void handle_iostream();

struct fileState {
   char* filename;
   int lineno;
};

struct fileState *files[8]; //Stack for file data
struct token *yytoken;

int top = -1; //Variable to keep track of size of stack
char *filename;
char *temp_sval;
int errors;




%}

intsuffix				([uU][lL]?)|([lL][uU]?)
fracconst				([0-9]*\.[0-9]+)|([0-9]+\.)
exppart					[eE][-+]?[0-9]+
floatsuffix				[fFlL]
chartext				([^'])|(\\.)
stringtext				([^"])|(\\.)

%%

"\n"					{ ++lineno; }
[\t\f\v\r ]+				{ /* Ignore whitespace. */ }

"/*"					{ skip_comment(); }
"//"					{ skip_until_eol(); }

{US}+({US}|{D})*	{ lexerr("Identifier not legal in 120++"); }

0[xX]{H}+{IS}?          { lexerr("Hex not legal in 120++\n"); }

0{O}+{IS}?              { lexerr("Octal not legal in 120++\n"); }

0(b|B)(0|1)+  		{ lexerr("Binary not legal in 120++\n"); }


"{"					{ return token('{'); }
"<%"					{ return token('{'); }
"}"					{ return token('}'); }
"%>"					{ return token('}'); }
"["					{ return token('['); }
"<:"					{ return token('['); }
"]"					{ return token(']'); }
":>"					{ return token(']'); }
"("					{ return token('('); }
")"					{ return token(')'); }
";"					{ return token(';'); }
":"					{ return token(':'); }
"..."					{ lexerr("Operator not legal in 120++"); }
"?"					{ return token('?'); }
"::"					{ return token(COLONCOLON); }
"."					{ return token('.'); }
".*"					{ return token(DOTSTAR); }
"+"					{ return token('+'); }
"-"					{ return token('-'); }
"*"					{ return token('*'); }
"/"					{ return token('/'); }
"%"					{ return token('%'); }
"^"					{ lexerr("Operator not legal in 120++"); }
"xor"					{ lexerr("Operator not legal in 120++"); }
"&"					{ return token('&'); }
"bitand"				{ return token('&'); }
"|"					{ return token('|'); }
"bitor"					{ return token('|'); }
"~"					{ return token('~'); }
"compl"					{ return token('~'); }
"!"					{ return token('!'); }
"not"					{ return token('!'); }
"="					{ return token('='); }
"<"					{ return token('<'); }
">"					{ return token('>'); }
"+="					{ return token(ADDEQ); }
"-="					{ return token(SUBEQ); }
"*="					{ return token(MULEQ); }
"/="					{ return token(DIVEQ); }
"%="					{ return token(MODEQ); }
"^="					{ return token(XOREQ); }
"xor_eq"				{ lexerr("Operator not legal in 120++"); }
"&="					{ lexerr("Operator not legal in 120++"); }
"and_eq"				{ lexerr("Operator not legal in 120++"); }
"|="					{ lexerr("Operator not legal in 120++"); }
"or_eq"					{ lexerr("Operator not legal in 120++"); }
"<<"					{ return token(SL); }
">>"					{ return token(SR); }
"<<="					{ lexerr("Operator not legal in 120++"); }
">>="					{ lexerr("Operator not legal in 120++"); }
"=="					{ return token(EQ); }
"!="					{ return token(NOTEQ); }
"not_eq"				{ return token(NOTEQ); }
"<="					{ return token(LTEQ); }
">="					{ return token(GTEQ); }
"&&"					{ return token(ANDAND); }
"and"					{ return token(ANDAND); }
"||"					{ return token(OROR); }
"or"					{ return token(OROR); }
"++"					{ return token(PLUSPLUS); }
"--"					{ return token(MINUSMINUS); }
","					{ return token(','); }
"->*"					{ return token(ARROWSTAR); }
"->"					{ return token(ARROW); }
<<EOF>>					{ return EOF; }

"asm"					{ lexerr("Keyword not legal in 120++"); }
"auto"					{ lexerr("Keyword not legal in 120++"); }
"bool"					{ return token(BOOL); }
"break"					{ return token(BREAK); }
"case"					{ return token(CASE); }
"catch"					{ lexerr("Keyword not legal in 120++"); }
"char"					{ return token(CHAR); }
"class"					{ return token(CLASS); }
"const"					{ return token(CONST); }
"const_cast"				{ lexerr("Keyword not legal in 120++"); }
"continue"				{ lexerr("Keyword not legal in 120++"); }
"default"				{ return token(DEFAULT); }
"delete"				{ return token(DELETE); }
"do"					{ return token(DO); }
"double"				{ return token(DOUBLE); }
"dynamic_cast"				{ lexerr("Keyword not legal in 120++"); }
"else"					{ return token(ELSE); }
"enum"					{ lexerr("Keyword not legal in 120++"); }
"explicit"				{ lexerr("Keyword not legal in 120++"); }
"export"				{ lexerr("Keyword not legal in 120++"); }
"extern"				{ lexerr("Keyword not legal in 120++"); }
"false"					{ return token(FALSE); }
"float"					{ return token(FLOAT); }
"for"					{ return token(FOR); }
"friend"				{ lexerr("Keyword not legal in 120++"); }
"goto"					{ lexerr("Keyword not legal in 120++"); }
"if"					{ return token(IF); }
"inline"				{ lexerr("Keyword not legal in 120++"); }
"int"					{ return token(INT); }
"long"					{ return token(LONG); }
"mutable"				{ lexerr("Keyword not legal in 120++"); }
"namespace"				{ return token(NAMESPACE); }
"new"					{ return token(NEW); }
"operator"				{ lexerr("Keyword not legal in 120++"); }
"private"				{ return token(PRIVATE); }
"protected"				{ lexerr("Keyword not legal in 120++"); }
"public"				{ return token(PUBLIC); }
"register"				{ lexerr("Keyword not legal in 120++"); }
"reinterpret_cast"			{ lexerr("Keyword not legal in 120++"); }
"return"				{ return token(RETURN); }
"short"					{ return token(SHORT); }
"signed"				{ return token(SIGNED); }
"sizeof"				{ return token(SIZEOF); }
"static"				{ lexerr("Keyword not legal in 120++"); }
"static_cast"				{ lexerr("Keyword not legal in 120++"); }
"struct"				{ return token(STRUCT); }
"switch"				{ return token(SWITCH); }
"template"				{ lexerr("Keyword not legal in 120++"); }
"this"					{ lexerr("Keyword not legal in 120++"); }
"throw"					{ lexerr("Keyword not legal in 120++"); }
"true"					{ return token(TRUE); }
"try"					{ lexerr("Keyword not legal in 120++"); }
"typedef"				{ lexerr("Keyword not legal in 120++"); }
"typeid"				{ lexerr("Keyword not legal in 120++"); }
"typename"				{ lexerr("Keyword not legal in 120++"); }
"union"					{ return token(UNION); }
"unsigned"				{ return token(UNSIGNED); }
"using"					{ return token(USING); }
"virtual"				{ lexerr("Keyword not legal in 120++"); }
"void"					{ return token(VOID); }
"volatile"				{ lexerr("Keyword not legal in 120++"); }
"wchar_t"				{ lexerr("Keyword not legal in 120++"); }
"while"					{ return token(WHILE); }

[a-zA-Z_][a-zA-Z_0-9]*			{ return check_identifier(yytext); }

"0"[xX][0-9a-fA-F]+{intsuffix}?		{ return token(INTEGER); }
"0"[0-7]+{intsuffix}?			{ return token(INTEGER); }
[0-9]+{intsuffix}?			{ return token(INTEGER); }

{fracconst}{exppart}?{floatsuffix}?	{ return token(FLOATING); }
[0-9]+{exppart}{floatsuffix}?		{ return token(FLOATING); }

"'"{chartext}*"'"			{ return token(CHARACTER); }
"L'"{chartext}*"'"			{ return token(CHARACTER); }

"#include"[ ]*\"[^\\"]+\"  		{ handle_include(); }

"#include"[ ]*" <cstdlib>"    		{ saw_cstdlib = true; }
"#include"[ ]*" <ctime>"      		{ saw_ctime = true; }
"#include"[ ]*" <cmath>"      		{ saw_cmath = true; }

"\""{stringtext}*"\""			{ return token(STRING); }
"L\""{stringtext}*"\""			{ return token(STRING); }
"#include"[ ]*"<iostream>"		{ saw_iostream = true; }
"#include"[ ]*"<string>"		{ saw_string = true; }
"#include"[ ]*"<fstream>"		{ saw_fstream = true; }
"using namespace std;"			{ saw_std = true; 
						if((saw_string || saw_iostream || saw_fstream) && !include_string) {
							handle_string();
							include_string = true;
						}
						if(saw_iostream && !include_io) {
							handle_iostream();
							include_io = true;
						}	
						if(saw_fstream && !include_fstream) {
							handle_fstream();
							include_fstream = true;;
						}
					}
							
"#include"[ ]*<[^>]+> 			{ lexerr("Include not legal in 120++\n"); }


.					{ fprintf(stderr, "%d: unexpected character `%c'\n", lineno, yytext[0]); }

%%

//Lexical error handling
void lexerr(char *s)
{
	errors++;
	fprintf(stderr, "%s:%d lexical error: %s: %s\n", filename, lineno, yytext, s);
	exit(1);
}


static
int yywrap()
{
 // Is the state stack empty?
    if(top == -1) 
      {
        return 1;
      } 
    else 
      {
        fclose(yyin);
        // Grab the previous input file from the state stack.
        yypop_buffer_state();
        pop_file();
        return 0;
      }   
}

void handle_fstream() {
	char *fname = strdup("include_fstream.h");
	FILE* input_file = fopen(fname, "r");
	if(!input_file) {
        	fprintf(stderr, "Unable to open include file %s: ", fname);
        	perror("");
        	exit(1);
    	}

   	 // Push flex's internal buffer state.
    	yypush_buffer_state(yy_create_buffer(input_file, YY_BUF_SIZE));
    	// Push "auxiliary" file data.
    	push_file(filename, lineno);
    	filename = fname;
    	lineno = 1;
	saw_fstream = false;
}

void handle_iostream() {
	char *fname = strdup("include_iostream.h");
	FILE* input_file = fopen(fname, "r");
	if(!input_file) {
        	fprintf(stderr, "Unable to open include file %s: ", fname);
        	perror("");
        	exit(1);
    	}

   	 // Push flex's internal buffer state.
    	yypush_buffer_state(yy_create_buffer(input_file, YY_BUF_SIZE));
    	// Push "auxiliary" file data.
    	push_file(filename, lineno);
    	filename = fname;
    	lineno = 1;
	saw_iostream = false;
}

void handle_string() {
	char *fname = strdup("include_string.h");
	FILE* input_file = fopen(fname, "r");
	if(!input_file) {
        	fprintf(stderr, "Unable to open include file %s: ", fname);
        	perror("");
        	exit(1);
    	}

   	 // Push flex's internal buffer state.
    	yypush_buffer_state(yy_create_buffer(input_file, YY_BUF_SIZE));
    	// Push "auxiliary" file data.
    	push_file(filename, lineno);
    	filename = fname;
    	lineno = 1;
	saw_string = false;
}

void handle_include()
{
    char *fname = strchr(yytext, '\"')+1;
    fname[strlen(fname)-1] = '\0';
    fname = strdup(fname);
    if (fname == NULL)
	exit(1);
    FILE* input_file = fopen(fname, "r");
    if(!input_file) {
        fprintf(stderr, "Unable to open include file %s: ", fname);
        perror("");
        exit(1);
    }

    // Push flex's internal buffer state.
    yypush_buffer_state(yy_create_buffer(input_file, YY_BUF_SIZE));
    // Push "auxiliary" file data.
    push_file(filename, lineno);
    filename = fname;
    lineno = 1;
}

char getChar(char *string){
	char *incPtr;
	memmove (string, string+1, strlen(string));
	string[strlen(string)-1] = '\0'; 
	incPtr = string;
	if(strlen(string) == 1)
		return string[0];
 	while(*incPtr != '\0') {
	 	if (*incPtr == '\\') {
	  		incPtr++;
			switch(*incPtr) {
				case 'n':
					return '\n';
				case 't':
					return '\t';
				case '\'':
					return '\'';
				case '\\':
					return '\\';
				case '\"':
					return '\"';
				case '0':
					return '\0';
				default:
					return *incPtr;
			}
		}
		incPtr++;
	}
	return 0;
}

void fix_sval(char *sval)
{
  int index = 0;
  char *incPtr;
  temp_sval = strdup(yytext);
  //Removes front double quote
  memmove (temp_sval, temp_sval+1, strlen(temp_sval)); 
  //Removes end double quote and replaces with null
  temp_sval[strlen(temp_sval)-1] = '\0'; 
  incPtr = temp_sval;
  //While loop increments through string and looks for escape characters
  //Unknown or illegal escape characters return an error
  while(*incPtr != '\0')
    {
      if (*incPtr == '\\')
	{
	  incPtr++;
	  index++; 	
	  switch(*incPtr)
	    {
	    case 'n' :
	      memmove(temp_sval+index, temp_sval+index+1, strlen(temp_sval)-index);
	      *(temp_sval+index-1) = '\n';
	      break;
	    case 't' :
	      memmove(temp_sval+index, temp_sval+index+1, strlen(temp_sval)-index);
	      *(temp_sval+index-1) = '\t';
	      break;
	    case '\'' :
	      memmove(temp_sval+index, temp_sval+index+1, strlen(temp_sval)-index);
	      *(temp_sval+index-1) = '\'';
	      break;
	    case '\\' :
	      memmove(temp_sval+index, temp_sval+index+1, strlen(temp_sval)-index);
	      *(temp_sval+index-1) = '\\';
	      break;
	    case '\"' :
	      memmove(temp_sval+index, temp_sval+index+1, strlen(temp_sval)-index);
	      *(temp_sval+index-1) = '\"';
	      break;
	    case '0' :
	      memmove(temp_sval+index, temp_sval+index+1, strlen(temp_sval)-index);
	      *(temp_sval+index-1) = '\0';
	      break;
	    default:
	      lexerr("Unknown escape sequence\n");
	      break;
	      
	    }
	}
      incPtr++;
      index++;
    }
  yytoken->sval = temp_sval; //Store "fixed" sval in token struct
  yytoken->valType = SVAL;
}


int token(int intCode)
{
   yytoken = (struct token *)malloc(sizeof(struct token));
   if (yytoken == NULL)
     exit(1);
   yytoken->category = intCode;
   yytoken->text = strdup(yytext);
   yytoken->lineno = lineno;
   yytoken->filename = strdup(filename);
   switch(intCode) {
	case INTEGER:
		yytoken->ival = atoi(yytext);
		break;
	case FALSE:
		yytoken->ival = 0;
		break;
	case TRUE:
		yytoken->ival = 1;
		break;
	case CHARACTER:
		yytoken->ival = getChar(yytext);
	default:
		break;
	}
   yytoken->sval = NULL;
   yylval.treenode = alcnode(yytoken); //Creates a tree node from yytoken
   return intCode;		       //to be used in tree for nonterminal
}

void push_file(char *filename, int lineno)
{
   if(top == 7)
     {	
        printf("Stack is full\n");
	exit(1);
     }
   else
     {
       top++;
       files[top] = (struct fileState *)malloc(sizeof(struct fileState));
       if (files[top] == NULL)
	exit(1);
       files[top]->filename = strdup(filename);
       files[top]->lineno = lineno;
     }
}

void pop_file()
{
   if(top == -1)
     {
        printf("Stack is empty\n");
	exit(1);
     }
   else
     {
	filename = files[top]->filename;
        lineno = files[top]->lineno;
	top--;
     }
}

/*
 * We use this routine instead a lex pattern because we don't need
 * to save the matched comment in the `yytext' buffer.
 */
static void
skip_comment(void)
{
	int c1, c2;

	c1 = input();
	c2 = input();

	while (c2 != EOF && !(c1 == '*' && c2 == '/')) {
		if (c1 == '\n')
			++lineno;
		c1 = c2;
		c2 = input();
				
		}
	
	
}

/*
 * See the previous comment for an explanation.
 */
static void
skip_until_eol(void)
{
	int c;

	while ((c = input()) != EOF && c != '\n')
		;
	++lineno;
}

/*
 *Skips include statements
 */


/*
 * Type name table.
 * Replace this with a hash table, when you get a chance.
 */
struct typenametable_entry {
   char *name;
   int category;
   struct typenametable_entry *next;
   } *head;

int typenametable_lookup(char *s)
{
   struct typenametable_entry *tmp = head;
   //If type name is found in table, creates token with class name 
   while (tmp) {
      if (!strcmp(s, tmp->name))
        { 
           yytext = tmp->name;
           token(tmp->category);
           return tmp->category;
        }
      tmp = tmp->next;
   }
   //If it's not in the table it's just an identifier and treated as so
   token(IDENTIFIER);
   return IDENTIFIER;
}

int typenametable_insert(char *s, int cat)
{
   struct typenametable_entry *tmp = head;
   while (tmp) {
      if (!strcmp(s, tmp->name)) {
	printf("TEST\n");
         fprintf(stderr,
	    "warning, %s is already in the typename table as a %d\n",
	    s, tmp->category );
         return tmp->category;
         }
      tmp = tmp->next;
   }
   tmp = calloc(1, sizeof (struct typenametable_entry));
   if(!tmp) { fprintf(stderr, "tnti out of memory\n"); return -1; }
   tmp->name = strdup(s);
   tmp->category = cat;
   tmp->next = head;
   head = tmp;
   return 0;
}

/*
 * Free's up type name table after each file parse
*/
void freetypenametable()
{
   struct typenametable_entry *tmp;	
   while (head != NULL)
     {
	tmp = head;	
	head = head->next;	
	free(tmp->name);
        free(tmp);
     }
}

static int
check_identifier(char *s)
{
	return typenametable_lookup(s);
}


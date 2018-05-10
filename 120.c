/*
 *This file handles the initial file opening and list operations.
 *It creates the list, fills it and adds new nodes. It then prints
 *the list when EOF is found on the last file. 
 */
#define _GNU_SOURCE
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "symtable.h"
#include "tree.h"
#include "type.h"
#include "intermediate.h"



extern FILE *yyin;
extern FILE *yyout;
extern int yyparse(void);
extern char* filename;
extern struct tree *root;
extern void freetypenametable(void);
extern struct symTable *global;
extern struct symTable *current;
extern struct symTable *constant;
extern bool include_io;
extern bool include_fstream;
extern bool include_string;

char *removeext(char* mystr);
int offset = 0;
int labels = 0;
enum region region = GLOBAL_R;

int main(int argc, char** argv)
{
  int filenum = 1; //Variable used to count number of files parsed from command line
  if (argc < 2)
    {
      printf("Please enter an input file.\n");
      exit(1);
    }
  /* Initial file opening handled in main. All other include file opening
   * handled in lex file by handle_include
   * While loop parses every file included on command line
   */	
  while(filenum < argc)
   {  
  	filename = argv[filenum];
  	yyin = fopen(argv[filenum], "r");
  	if (!yyin)
    	{
      		fprintf(stderr, "Could not open %s\n", argv[filenum]);
      		exit(1);
   	 }
	printf("File opened: %s\n", filename);
  	global = newTable(GLOBAL_R,  10000); 	
	constant = newTable(CONST_R, 1000);			     
  	current = global;
  	yyparse();
	//preorder(root, 0);	
  	populateSymTable(root);
	typecheck(root);
	gen(root);

	char *outfile = removeext(filename);
	asprintf(&outfile, "%s.ic", outfile);
	FILE *ic = fopen(outfile, "w");
	fprintf(ic, ".data\n");
	int i;
	for(i = 0; i < global->tSize; i++) {
		if (global->symbols[i] != NULL && global->symbols[i]->type->basetype != FUNCTION_T){
			fprintf(ic, "global: %s\n", global->symbols[i]->name);
		}
	}
	fprintf(ic, ".code\n");
	printCodeList(ic, root->code);
	fclose(ic);
    	/* Reset flags and variables */
 	include_io = false;
	include_fstream = false;
	include_string = false; 
	labels = 0;
	offset = 0;
	region = GLOBAL_R;
	/* Free typename table */
  	freetypenametable(); 
  	filenum++;
  }
  return 0;
}

char *removeext(char* mystr) {
    char *retstr;
    char *lastdot;
    if (mystr == NULL)
         return NULL;
    if ((retstr = malloc (strlen (mystr) + 1)) == NULL)
        return NULL;
    strcpy (retstr, mystr);
    lastdot = strrchr (retstr, '.');
    if (lastdot != NULL)
        *lastdot = '\0';
    return retstr;
}



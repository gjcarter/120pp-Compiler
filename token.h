#ifndef TOKEN_H
#define TOKEN_H

#include "type.h"
#define IVAL 1
#define DVAL 2
#define SVAL 3
#define MEM 12 //Exit code for out of memory

struct address;

//From http://www2.cs.uidaho.edu/~jeffery/courses/445/hw1.html
struct token {
  int category;   /* the integer code returned by yylex */
  char *text;     /* the actual string (lexeme) matched */
  int lineno;     /* the line number on which the token occurs */
  char *filename; /* the source file in which the token occurs */
  int valType;    /* used for print function to check if any val has been set */
  int ival;       /* if you had an integer constant, store its value here */
  double dval;    /* for real constants, store binary value here */
  char *sval;
  struct address place;      
};
struct tokenlist {
  struct token *t;
  struct tokenlist *next;
};

//Frees up memory when out of memory or at end of program
void freeMemory();

struct tokenlist *listhead;

#endif

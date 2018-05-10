#include <stdarg.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "token.h"
#include "nonterm.h"
#include "type.h"

struct tree {
   short label;			/* what production rule this came from */
   short nkids;			/* how many children it really has */
   struct token *leaf;
   struct list *code;
   struct address place;
   struct tree *parent;
   struct tree *kids[9];        /* array of children, size varies 0..k */
   				
};

//Allocates tree for given number of children
struct tree *alctree(int label, int nkids, ...);

/* This function allocates a new node and is used for nonterminals.
 * When a nonterminal is found token() allocates a new node and is 
 * assigned to a tree node using this function. 
 */
struct tree *alcnode(struct token *t);

//Function to print the tree
void printer(struct tree *t, int depth);

//Traverses tree preorder and calls print function for each node
void preorder(struct tree *t, int depth);

void semantic_error(char *s, struct tree *t);

/*Big awful functtion to get the string
 *based on the production rule int code. 
 *Case number based on int code
 */
char* humanreadable(int rule);
struct tree *getNode(struct tree *t, int child);

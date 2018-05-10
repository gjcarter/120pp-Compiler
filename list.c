#include "list.h"
#include <stdio.h>
#include <stdlib.h>

/* Add node to back of list */
void alclistback(void *data, struct list **list){
	
	if(*list == NULL){
		*list = malloc(sizeof(struct list));
		(*list)->data = data;
		(*list)->next = NULL;
		
		
	}
	else{
		struct list *temp = *list;
		while(temp->next != NULL)	
			temp = temp->next;
		struct list *new = malloc(sizeof(struct list));
		new->data = data;
		new->next = NULL;
		temp->next = new;
	}
		
}


/* Add node to front of list */
void alclistfront(void *data, struct list **list){
	
	if(*list == NULL){
		*list = malloc(sizeof(struct list));
		(*list)->data = data;
		(*list)->next = NULL;
	}
	else{
		struct list *temp = *list;
		struct list *new = malloc(sizeof(struct list));
		new->data = data;
		new->next = temp;
		(*list) = new;
	}
		
}

/* Returns number of nodes in list */
int paramNum(struct list *list){
	int num = 0;
	struct list *temp = list;
	
	if(list == NULL)
		return 0;
	else {
		while(temp != NULL) {
			temp = temp->next;
			num++;	
		}
		return num;		
	}		
}

struct list *listconcat(struct list *list1, struct list *list2){
	if(list1 == NULL)
		return list2;	
	struct list *temp = list1;
	while (temp->next != NULL)
		temp = temp->next;
	temp->next = list2;
	return list1;;
}

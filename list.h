#ifndef LIST_H
#define LIST_H

struct list {
	void *data;
	struct list *next;	
};

void alclistback(void *data, struct list **list);
void alclistfront(void *data, struct list **list);
int listSize(struct list *list);
struct list *listconcat(struct list *list1, struct list *list2);

#endif

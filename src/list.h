#ifndef _List_H_
#define _LIST_H_

typedef struct _Node
{
	struct _Node *next;
	struct _Node *prev;
	void *data;
} _Node;
typedef struct _Node Node;

typedef struct _List{
	Node *head;
	Node *tail;
} _List;
typedef struct _List List;


/* Public Facing Function Calls */
list *init_list();

int list_remove(List *list_to_remove, void *data);
void list_add(List *list_to_add, void *data);
List *init_list();
void *list_pop(List *list_to_pop);

#endif

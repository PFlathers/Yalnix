#ifndef _LIST_H_
#define _LIST_H_

typedef struct _Node
{
	struct _Node *next;
	struct _Node *prev;
	int id = -1;
	void *data;
} _Node;
typedef struct _Node Node;

typedef struct _List{
	Node *head;
	Node *tail;
} _List;
typedef struct _List List;


/* Public Facing Function Calls */
int list_remove(List *list_to_remove, void *data);
void list_add(List *list_to_add, void *data);
void list_add2(List *list_to_add, void *data, int id);
List *init_list();
void *list_pop(List *list_to_pop);
int list_count(List *list_to_count);

#endif

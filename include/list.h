#ifndef _LIST_H_
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
        int count;
} _List;
typedef struct _List List;


/* Public Facing Function Calls */
int list_remove(List *list_to_remove, void *data);
void list_add(List *list_to_add, void *data);
List *init_list();
void list_push(List *list_to_push, void *data);
void *list_pop(List *list_to_pop);
int list_count(List *list_to_count);
int list_remove_non_delete(List *list_to_remove, void *data);
#endif

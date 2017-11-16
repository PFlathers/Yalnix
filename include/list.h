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

//creates a list
List *init_list();

// adds data onto the end of the list
void list_add(List *list_to_add, void *data);
void list_add_messy(List *list_to_add, void *data);


// add data onto the begining of the list
void list_push(List *list_to_push, void *data);

// removes and returns data from the begining of the list
void *list_pop(List *list_to_pop);

// returns the size of the list
int list_count(List *list_to_count);

// removes the data from the list
int list_remove(List *list_to_remove, void *data);

// removes and frees data form the list
int list_remove_non_delete(List *list_to_remove, void *data);
#endif

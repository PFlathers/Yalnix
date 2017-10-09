
#ifndef _LIST_H_
#define _LIST_H_

/* Node for a linked list/queue. has a void pointer so you can put whatever data
 * you want in it.
 */
typedef struct _Node
{
        struct _Node *next;
        struct _Node *prev;
        void *data;
} _Node;
typedef struct _Node node;


/* Define a list type so that we don't have to mess around with
 * nodes in our implementation
 */
typedef struct _List{
	node head;
} _List;
typedef struct List


/* Public Facing Function Calls */
int list_remove(list list_to_remove, void *data);
void list_add(list *list_to_add, void *data);



#endif

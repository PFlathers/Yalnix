
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



/* Public Facing Function Calls */



#endif

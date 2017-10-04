/* Node for a linked list/queue. has a void pointer so you can put whatever data
 * you want in it.
 */
typedef struct _Node
{
        struct _Node *next;
        struct _Node *prev;
        void *data;
} __Node;
typedef struct _Node Node;

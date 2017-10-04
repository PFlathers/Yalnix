/*
 */
typedef struct _LOCK
{
        int id; // id of lock
        int claimed; // 0 for free, 1 for claimed
        unsigned int proc_id; //who has the lock.
	// question: shouldn't it just be Node * waiters? 
        struct _Node *waiters; //who has asked for the lock and will get it next
} __LOCK;
typedef struct _LOCK lock;

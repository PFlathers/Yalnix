#ifndef _LOCK_H_
#define _LOCK_H_

#include "list.h"


typedef struct _LOCK
{
        int id; // id of lock
        int claimed; // 0 for free, 1 for claimed
        unsigned int proc_id; //who has the lock.
        struct node *waiters; //who has asked for the lock and will get it next
} __LOCK;
typedef struct _LOCK Lock;

/* Public Facing Function Calls */
Lock *lock_init();
int lock_aquire(Lock *lock);
int lock_release(Lock *lock);

#endif

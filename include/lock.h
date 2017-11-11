#ifndef _LOCK_H_
#define _LOCK_H_

#include "list.h"


typedef struct _LOCK
{
        int id; // id of lock
        int claimed; // 0 for free, 1 for claimed
        unsigned int proc_id; //who has the lock.
        List *waiters; //who has asked for the lock and will get it next
} __LOCK;
typedef struct _LOCK Lock;

/* Public Facing Function Calls */
int kernel_LockInit(int *lock_to_init);
int kernel_Acquire(int lock_id);
int kernel_Release(int lock_id);
int kernel_LockDestroy(int lock_id);

Lock *findLock(int lock_id);
#endif

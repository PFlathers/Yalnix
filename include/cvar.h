#ifndef _CVAR_H_
#define _CVAR_H_

#include "list.h"
#include "lock.h"


typedef struct _CVAR
{
        int id; //id of the cvar
        int claimed; // 0 if not claimed. 1 if claimed
        unsigned int proc_id;
        List *waiters; // who is waiting on signal.
        Lock *lock;
} _CVAR;
typedef struct _CVAR cvar;

/* Public Facing Function Calls */

int CvarInit(Cvar *cvar_to_init);

int CvarDestory(int cvar_id);

int CvarSignal(int cvar_id);

int CvarBroadcast(int cvar_id);

int CvarWait(int cvar_id, int lock_id);
#endif

#ifndef _CVAR_H_
#define _CVAR_H_

#include "list.h"
#include "lock.h"


typedef struct _CVAR
{
        int id; //id of the cvar
        List *waiters; // who is waiting on signal.
        Lock *lock;
} _CVAR;
typedef struct _CVAR Cvar;

/* Public Facing Function Calls */

int kernel_CvarInit(int*);

int kernel_CvarSignal(int cvar_id);

int kernel_CvarBroadcast(int cvar_id);

int kernel_CvarWait(int cvar_id, int lock_id);

Cvar *findCvar(int cvar_id);
#endif

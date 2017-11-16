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

//Initializes a cvar and sets the passed pointer to the cvar's id
int kernel_CvarInit(int*);

// Pops a waiting process and schedules it to run
int kernel_CvarSignal(int cvar_id);

// Pops all the waiting process and schedule them to run
int kernel_CvarBroadcast(int cvar_id);

/* Set the current proc as blocked and wait for a condition to be met
 * before running again.
 */
int kernel_CvarWait(int cvar_id, int lock_id);

#endif

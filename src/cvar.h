#ifndef _CVAR_H_
#define _CVAR_H_

#include "list.h"


typedef struct _CVAR
{
        int id; //id of the cvar
        int claimed; // 0 if not claimed. 1 if claimed
        struct node *waiters; // who is waiting on signal.
} _CVAR;
typedef struct _CVAR cvar;

/* Public Facing Function Calls */

int cvar_init(Cvar *cvar_to_init);

int cvar_destroy(Cvar *cvar_to_destroy);

int cvar_signal(Cvar *cvar_to_signal);

int cvar_broadcast(Cvar *cvar_to_broadcast);

int cvar_wait(Cvar *cvar_to_wait);
#endif

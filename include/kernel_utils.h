#ifndef _UTILS_
#define _UTILS_
#include "cvar.h"
#include "pipe.h"
#include "lock.h"

Cvar *kernel_findCvar(int cvar_id);

Lock *kernel_findLock(int lock_id);

Pipe *kernel_findPipe(int lock_id);

void free_pagetables(pcb* myproc);

#endif

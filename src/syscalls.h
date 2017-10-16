#ifndef _SYSCALLS_H_
#define _SYSCALLS_H_


// these includes will provide the syscalls for their respective data
// structures instead of them being defined here. 
#include "lock.h"
#include "pipe.h"
#include "cvar.h"
#include "tty.h"



/* Public Facing Function Calls */

int Fork();

int Exec(char *filename, char **argvec);

void Exit(int status);

int Wait(int * status_ptr);

int GetPid();

int Brk(void *addr);

int Delay(int clock_ticks);

int Reclaim(int id);
#endif

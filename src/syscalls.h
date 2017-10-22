#ifndef _SYSCALLS_H_
#define _SYSCALLS_H_

#include <hardware.h>

// these includes will provide the syscalls for their respective data
// structures instead of them being defined here. 
#include "pipe.h"
#include "tty.h"



/* Public Facing Function Calls */

int kernel_Fork();

int kernel_Exec(char *filename, char **argvec);

void kernel_Exit(int status);

int kernel_Wait(int * status_ptr);

int kernel_GetPid();

int kernel_Brk(void *addr);

int kernel_Delay(UserContext *user_context, int clock_ticks);

int kernel_Reclaim(int id);
#endif

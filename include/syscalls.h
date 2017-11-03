#ifndef _SYSCALLS_H_
#define _SYSCALLS_H_

#include <hardware.h>

// these includes will provide the syscalls for their respective data
// structures instead of them being defined here. 
#include "pipe.h"
#include "tty.h"
#include "pcb.h"


/* Macros */
#define FRAME_TO_PAGE(n) ((u_long) (((int)(n) * PAGESIZE) >> PAGESHIFT))
#define PAGE_TO_FRAME(n) (((n) << PAGESHIFT) / PAGESIZE)


/* Public Facing Function Calls */


/*------------------------------------------------- kernel_Fork -----
|  Function kernel_Fork
|
|  Description:  Fork is the only way new processes are created in Yalnix.
|  The memory image of the new process (the child) is a copy of that of 
|  the process calling Fork (the parent). When the Fork call completes, 
|  both the parent process and the child process return (separately) from 
|  the kernel call as if they had been the one to call Fork, since the 
|  child is a copy of the parent. The only distinction is the fact that 
|  the return value in the calling (parent) process is the process ID of the 
|  new (child) process, while the value returned in the child is 0. If, 
|  for any reason, the new process cannot be created, this syscall instead 
|  returns the value ERROR to the calling process.
|
|  Parameters:
|      1) UserContext *user_context (IN/OUT) -- user context of currently
| 				running process - used to bootstrap the parent's nad child's
| 				UC to get them on the same page; may be updated at context
|				switching
|
|  Returns:  this is hte only function that returns _TWICE_. Separately
| 			as a kid (return value == 0) and as a parent (return value ==
| 			child's id). 
*-------------------------------------------------------------------*/
int kernel_Fork(UserContext *user_context);

int kernel_Exec(UserContext*, char *filename, char **argvec);

void kernel_Exit(int status, UserContext *uc);


void free_pagetables(pcb* myproc);
/*------------------------------------------------- Kernel_Wait -----
 |  Function kernel_Wait
 |
 | Description:  Collect the process ID and exit status returned by a child
 | process of the calling program. When a child process Exits, its exit 
 | status information is added to a FIFO queue of child processes not 
 | yet collected by its specific parent. After the Wait call, this child 
 | process information is removed from the queue. If the calling process 
 | has no remaining child pro- cesses (exited or running), ERROR is returned. 
 | Otherwise, if there are no exited child pro- cesses waiting for collection 
 | by this calling process, the calling process is blocked until its next child 
 | calls exits or is aborted. On success, the process ID of the child process is 
 | returned and its exit status is copied to the integer referenced by the 
 | status ptr argument. On any error, this call instead returns ERROR.
 |
 |  Parameters:
 |      status_ptr (OUT or IN/OUT): exit status of the returned chiled 
 |									copied to the integer 
 |                   
 |
 |  Returns:  int with returing childs PID
 *-------------------------------------------------------------------*/
int kernel_Wait(int * status_ptr, UserContext *uc);

int kernel_GetPid();

int kernel_Brk(void *addr);

int kernel_Delay(UserContext *user_context, int clock_ticks);

int kernel_Reclaim(int id);
#endif

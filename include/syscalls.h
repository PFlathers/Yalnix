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


/*------------------------------------------------- Kernel_Wait -----
 |  Function kernel_Wait
 |
 |  Description:  Collect the process ID and exit status returned by a child
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
int kernel_Wait(int * status_ptr);

int kernel_GetPid();

int kernel_Brk(void *addr);

int kernel_Delay(UserContext *user_context, int clock_ticks);

int kernel_Reclaim(int id);
#endif

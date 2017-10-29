#ifndef _SYSCALLS_H_
#define _SYSCALLS_H_

#include <hardware.h>

// these includes will provide the syscalls for their respective data
// structures instead of them being defined here. 
#include "pipe.h"
#include "tty.h"



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


/*------------------------------------------------- kernel_Exec -----
|  Function kernel_Exec
|
|  Description:  Replace the currently running program in the calling process’s 
|  memory with the program. stored in the file named by filename. 
| The argument argvec points to a vector of arguments to pass to the new 
| program as its argument list. When the new program begins running, 
| its argv[argc] is NULL. By convention the first argument in the argument 
| list passed to a new program (argvec[0]) is also the name of the new 
| program to be run, but this is just a convention; the actual file name 
| to run is determined only by the filename argument. On success, 
| there is no return from this call in the calling program, and instead, 
| the new program begins executing in this process at its entry point, and its 
| main(argc, argv) routine is called. On failure, if the calling process has 
| not been destroyed already, this call returns ERROR and does not run the 
| new program. (However, if the kernel has already torn down the caller 
| before encountering the error, then there is no process to return to!)
|
|  Parameters:
|      UserContext *uc (IN/OUT) -- user context of currently
| 				running process - used to bootstrap the new procees
| 				with same vector code and address
|	   char *filename (IN) - time of the program to load
|	   char **argvec (IN) - list of arguments to program in "filename" 		
|
|  Returns:  SUCCESS if the new program started, ERROR otherwise
*-------------------------------------------------------------------*/
int kernel_Exec(UserContext *uc, char *filename, char **argvec);

void kernel_Exit(int status);


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

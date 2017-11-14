#ifndef _SYSCALLS_H_
#define _SYSCALLS_H_

#include <hardware.h>

// these includes will provide the syscalls for their respective data
// structures instead of them being defined here. 
#include "pipe.h"
#include "tty.h"
#include "pcb.h"
#include "cvar.h"
#include "lock.h"


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
| 			as a kid (return value == 0) and as a parent 
|                       (return value == child's id). 
*-------------------------------------------------------------------*/
int kernel_Fork(UserContext *user_context);


/* --------------------------kernel_Exec-----------------------------------
 * Loads a program into the current process and begins execution of that program.
 *
 | Description: Replace the currently running program in the calling process’s 
 |      memory with the program stored in the file named by filename. The argument 
 |      argvec points to a vector of argu- ments to pass to the new program as 
 |      its argument list. The strings pointed to by the entries in argv are copied 
 |      from the strings pointed to by the argvec array passed to Exec, and argc 
 |      is a count of entries in this array before the first NULL entry, which 
 |      terminates the argument list. When the new program be- gins running, its 
 |      argv[argc] is NULL. By convention the first argument in the argument list 
 |      passed to a new program (argvec[0]) is also the name of the new program 
 |      to be run, but this is just a convention; the actual file name to run is 
 |      determined only by the filename argument. On success, there is no return 
 |      from this call in the calling program, and instead, the new program begins 
 |      executing in this process at its entry point, and its main(argc, argv) 
 |      routine is called as indicated above. On failure, if the calling process 
 |      has not been destroyed already, this call returns ERROR and does not 
 |      run the new program.
 |
 * Parameters:
 *      UserContext *uc: the current user context
 *      char *filename: the file that we have to execute
 *      char *argvec; the arguements that we are going to pass to LoadProg so that we 
 *              can load the program and pass it the proper arguements.
 |
 |
 * Returns:} Sucess if we sucessfully exec the program. returns ERORR if there is
 * a problem with Load Prog. Common error is that the file doesnt exist...
 *------------------------------------------------------------------------*/
int kernel_Exec(UserContext*, char *filename, char **argvec);


/*-------------------------Kernel_Exit----------------------------------------
 | Function: kernel_Exit
 | Description: The current process is terminated, the integer status value is 
 |      saved for possible later collection by the parent process on a call to 
 |      Wait. All resources used by the calling process are freed, except 
 |      for the saved status information. This call can never return. When a 
 |       process exits or is aborted, if it has children, they should continue 
 |       to run normally, but they will no longer have a parent. 
 |
 | Parameters:
 |      status: the exit status of the process.
 |      uc: the current user context.
 *-------------------------------------------------------------------------*/
void kernel_Exit(int status, UserContext *uc);


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
 |		copied to the integer 
 |                   
 |
 |  Returns:  int with returing childs PID
 *-------------------------------------------------------------------*/
int kernel_Wait(int * status_ptr, UserContext *uc);


/*------------------------------------------------- kernel_GetPid -----
 |  Function kernel_GetPid
 |
 | Description: returns curr proc PID
 |
 |  Parameters:
 |      none
 |
 |  Returns:  int with current PID
 *-------------------------------------------------------------------*/
int kernel_GetPid();

/*------------------------------------------------- kernel_Brk -----
 |  Function kernel_Brk
 |
 | Description:  sets the operating system’s idea of the lowest location 
 | not used by the program (called the “break”) to addr (rounded up to 
 | the next multiple of PAGESIZE bytes). This call has the effect of allocating 
 | or deallocating enough memory to cover only up to the specified address. 
 | Locations not less than addr and below the stack pointer are not in the 
 | address space of the process and will thus cause an exception if accessed. 
 | The value 0 is returned on success. If any error is encountered (for example, 
 | if not enough memory is available or if the address addr is invalid), the 
 | value ERROR is returned.
 |
 |  Parameters:
 |      void *addr (IN): address to set the brk to
 |                   
 |
 |  Returns:  SUCCESS if ok, ERROR otherwise
 *-------------------------------------------------------------------*/
int kernel_Brk(void *addr);

/*------------------------------------------------- kernel_Delay -----
 |  Function kernel_Delay
 |
 | Description:  The calling process is blocked until clock ticks clock 
 | interrupts have occurred after the call. Upon completion of the delay, 
 | the value 0 is returned. If clock ticks is 0, return is immediate. If 
 | clock ticks is less than 0, time travel is not carried out, and ERROR 
 | is returned instead.
 |
 |  Parameters:
 |      UserContext *user_context (IN): current user context
 |      int clock_ticks (IN): how long to delay a process
 |                   
 |
 |  Returns:  SUCCESS if delay completed, ERROR otherwise
 *-------------------------------------------------------------------*/
int kernel_Delay(UserContext *user_context, int clock_ticks);


/*------------------------------------------------- kernel_Reclaim -----
 |  Function kernel_Reclaim
 |
 | Description: Destroy the lock, condition variable, or pipe indentified by 
 | id, and release any associated resources. In case of any error, the value 
 | ERROR is returned.
 |
 |  Parameters:
 |      int id (IN): ID of the resource to be reclaimed
 |                   
 |
 |  Returns:  SUCCESS if reclaimed, ERROR otherwise
 *-------------------------------------------------------------------*/
int kernel_Reclaim(int id);


#endif

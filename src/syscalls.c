#include "globals.h"
#include "syscalls.h"
#include "pipe.h"

#include "pcb.h"
#include "kernel.h"


int kernel_Fork()
{
	return 0;
}

int kernel_Exec(char *filename, char **argvec)
{
	return 0;
}

void kernel_Exit(int status)
{
}

int kernel_Wait(int * status_ptr)
{
	return 0;
}

int kernel_GetPid()
{
	return 0;
}

int kernel_Brk(void *addr)
{
	return 0;
}

int kernel_Delay(UserContext *user_context, int clock_ticks)
{
	if (clock_ticks == 0)
		return SUCCESS;
	if (clock_ticks < 0)
		return ERROR;
	// curr_proc->start_count = clock_ticks;
	// curr_proc->timeflag = 1;

	// set up block handler for curr process
	curr_proc->block->is_active = ACTIVE;
	curr_proc->block->type = DELAY;
	curr_proc->block->stats.count = clock_ticks;


	//Interupts off
	//list_remove(&ready_procs, (void *) curr_proc);
	list_add(blocked_procs, (void *) curr_proc);

	if (list_count(ready_procs) < 1){
		TracePrintf(2, "no items to switch to! ");
		exit(FAILURE);
	} else {
		if (goto_next_process(user_context, 0) != SUCCESS){
			TracePrintf(3, "Delay: goto_next_process failed with error");
		}
	}

	return SUCCESS;
	
	//Interupts on

	/* As part of the Scheduler:
	 * Move from ready list to blocked list.
	 * for each process in blocked list:
	 *	if(pcb->timeflag == 1):
	 *		if  pcb->start_count > 0:
	 *			start_count --;
	 *		else:
	 *			move pcb from blocked to ready.
	 */

	/* Round Robin for this only has a quantum of 1 clock tick. since the
	 * kernel is uninteruptable we wont be counting clock ticks when we are
	 * scheduling.
	 */
}

/* 
 * Destroys the lock, cvar, or pipe identified by the id, and releases any
 * associated resources. In case of error, the value ERROR is returned.
 */
int kernel_Reclaim(int id)
{
	//If id is cvar
	// CvarDestory(id);
	// // if id is lock
	// LockDestory(id);
	// // if id is pipe
	// PipeDestroy(id);
	return 0;
}

#include "globals.h"
#include "syscalls.h"
#include "pipe.h"
#include "lock.h"
#include "cvar.h"
#include "pcb.h"
#include "kernel.h"


int Fork()
{
	return 0;
}

int Exec(char *filename, char **argvec)
{
	return 0;
}

void Exit(int status)
{
}

int Wait(int * status_ptr)
{
	return 0;
}

int GetPid()
{
	return 0;
}

int Brk(void *addr)
{
	return 0;
}

int Delay(int clock_ticks)
{
	if (clock_ticks == 0)
		return 0;
	else if (clock_ticks < 0)
		return ERROR
	curr_proc->start_count = clock_ticks;
	curr_proc->timeflag = 1;


	//Interupts off
	list_remove(&ready_procs, (void *) curr_proc);
	list_add(&blocked_procs, (void *) curr_proc);
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


	return 0;
}

/* 
 * Destroys the lock, cvar, or pipe identified by the id, and releases any
 * associated resources. In case of error, the value ERROR is returned.
 */
int Reclaim(int id)
{
	//If id is cvar
	CvarDestory(id);
	// if id is lock
	LockDestory(id);
	// if id is pipe
	PipeDestroy(id);
	return 0;
}

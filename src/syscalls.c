#include "globals.h"
#include "syscalls.h"
#include "pipe.h"
#include "lock.h"
#include "cvar.h"


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

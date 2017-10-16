#include "globals.h"
#include "syscalls.h"

/* Here we just have prototypes for Cvar functions
 */

//create a cvar for the cvar_to_init pointer
int CvarInit(Cvar *cvar_to_init)
{
	return 0;
}

//frees all the data in the cvar and removes it from the linked list.
int CvarDestory(int cvar_id)
{
	return 0;
}

//let the next waiter know its time to run.
int CvarSignal(int cvar_id)
{
	return 0;
}

//let all the waiter know.
int CvarBroadcast(int cvar_id)
{
	return 0;
}

//wait until we get a cvar signal or broadcast.
int CvarWait(int cvar_id, int lock_id)
{
	return 0;
}

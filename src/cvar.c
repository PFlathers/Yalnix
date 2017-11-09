#include "globals.h"
#include "syscalls.h"
#include "kernel.h"
#include "cvar.h"
#include "list.h"

/* Here we just have prototypes for Cvar functions
 */

//create a cvar for the cvar_to_init pointer
int CvarInit(int *cvar_idp)
{
        Cvar *cvar_to_init = (Cvar*) malloc(sizeof(Cvar));
        cvar_to_init->id = available_lock_id;
        available_lock_id++;
        cvar_to_init->proc_id = 0;
        cvar_to_init->claimed = 0;
        init_list(cvar_to_init->wait);
        list_add(cvars, cvar_to_init);

        *cvar_idp = cvar_idp;
	return 0;
}

//frees all the data in the cvar and removes it from the linked list.
int CvarDestory(int cvar_id)
{
	return 0;
}

//let the next waiter know its time to run.
//allow only the next waiter to get it
int CvarSignal(int cvar_id)
{
	return 0;
}

//let all the waiter know.
//allow any waiter to get it.
int CvarBroadcast(int cvar_id)
{
	return 0;
}

//wait until we get a cvar signal or broadcast.
int CvarWait(int cvar_id, int lock_id)
{
        /*
         * move the process from ready procs to blocked.
         * release the lock ided by lock_id
         */
	return 0;
}

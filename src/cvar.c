#include "globals.h"
#include "syscalls.h"
#include "kernel.h"
#include "cvar.h"
#include "list.h"
#include "lock.h"


/*create a new Condition variable and sets *cvar_idp to the id of the cvar
 * so that the user knows what the id of the Cvar is.
 */
int kernel_CvarInit(int *cvar_idp)
{
        TracePrintf(8, "Creating a new Cvar\n");
        Cvar *cvar_to_init = (Cvar*) malloc(sizeof(Cvar));
        cvar_to_init->id = available_lock_id;
        available_lock_id++;
        cvar_to_init->waiters = init_list();
        list_add(cvars, cvar_to_init);

        *cvar_idp = cvar_to_init->id;

        TracePrintf(8, "Finished creating Cvar\n");
	return 0;
}
/*
 * Pops the next process to run from waiters.
 *  remove the process from blocked procs
 *  add the process to ready procs.
 */
int kernel_CvarSignal(int cvar_id)
{
        TracePrintf(8, "Signaling Cvar %d\n", cvar_id);
        Cvar *my_cvar = (Cvar*) kernel_findCvar(cvar_id);

        //Check there is a process to signal
        if (my_cvar->waiters->count == 0){
                return ERROR;
        }

        // move process to where it should be
        pcb *p = list_pop(my_cvar->waiters);
        list_remove(blocked_procs, (void*) p);
        list_add(ready_procs, (void*) p);

        TracePrintf(8, "Done signaling\n");
	return SUCCESS;
}

/* let all the waiters know that a particular condition has been met.
 * Moves all waiters from the blocked procs to ready procs
 */
int kernel_CvarBroadcast(int cvar_id)
{
        TracePrintf(8, "Broadcasting Cvar %d\n", cvar_id);
        Cvar *my_cvar = (Cvar*) kernel_findCvar(cvar_id);

        //Making sure the cvar exits
        if (my_cvar->waiters->count == 0){
                return ERROR;
        }
        
       //let all the cvars know 
        Node *temp = my_cvar->waiters->head;
        pcb *p;
        while(temp != NULL){
                p = list_pop(my_cvar->waiters);
                list_remove(blocked_procs, (void*) p);
                list_add(ready_procs, (void*) p);
                temp = my_cvar->waiters->head;
        }

        TracePrintf(8, "Done Broadcasting\n");
	return SUCCESS;
}

/*wait until we get a cvar signal or broadcast.
 *
 * move the process from ready procs to blocked.
 * release the lock ided by lock_id
 */
int kernel_CvarWait(int cvar_id, int lock_id)
{
        TracePrintf(8, "Begin Conditional Wait\n\tCvar:%d\n\tLock:%d\n", cvar_id, lock_id);
        Cvar *my_cvar = (Cvar*) kernel_findCvar(cvar_id);


        Lock *my_lock = (Lock*) kernel_findLock(lock_id);

        // Check that I currently have the lock
        if (kernel_Release(lock_id) != SUCCESS){
                TracePrintf(3, "Process did not have the lock");
                return ERROR;
        }

        //move to blocked procs and to cvar waiters
        list_add(blocked_procs, curr_proc);
        list_add(my_cvar->waiters, curr_proc);

        TracePrintf(8, "Going to cvar wait. Switching process\n");
        goto_next_process(curr_proc->user_context, 0);

        TracePrintf(8, "Finished cvar waiting\n");
        return SUCCESS;

}

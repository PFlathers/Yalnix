#include "globals.h"
#include "syscalls.h"
#include "kernel.h"
#include "cvar.h"
#include "list.h"
#include "lock.h"

/* Here we just have prototypes for Cvar functions
 */

//create a cvar for the cvar_to_init pointer
int CvarInit(int *cvar_idp)
{
        Cvar *cvar_to_init = (Cvar*) malloc(sizeof(Cvar));
        cvar_to_init->id = available_lock_id;
        available_lock_id++;
        //cvar_to_init->proc_id = 0;
        //cvar_to_init->claimed = 0;
        cvar_to_init->wait = init_list();
        list_add(cvars, cvar_to_init);

        *cvar_idp = cvar_to_init->id;
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
        Cvar *my_cvar = findCvar(cvar_id);
        if (my_cvar->wait->count == 0){
                return ERROR;
        }
        pcb *p = list_pop(my_cvar->wait);
        list_remove(blocked_procs, (void*) p);
        list_add(ready_procs, (void*) p);

	return SUCCESS;
}

//let all the waiter know.
//allow any waiter to get it.
int CvarBroadcast(int cvar_id)
{
        Cvar *my_cvar = findCvar(cvar_id);
        if (my_cvar->wait->count == 0){
                return ERROR;
        }
        
        Node *temp = my_cvar->wait->head;
        pcb *p;
        while(temp != NULL){
                p = list_pop(my_cvar->wait);
                list_remove(blocked_procs, (void*) p);
                list_add(ready_procs, (void*) p);
                temp = my_cvar->wait->head;
        }
//
	return SUCCESS;
}

//wait until we get a cvar signal or broadcast.
int CvarWait(int cvar_id, int lock_id)
{
        /*
         * move the process from ready procs to blocked.
         * release the lock ided by lock_id
         */

        Cvar *my_cvar = findCvar(cvar_id);

        Lock *my_lock = findLock(lock_id);
        if (kernel_Release(lock_id) != SUCCESS){
                TracePrintf(3, "Process did not have the lock");
                return ERROR
        }
        list_remove(ready_procs, curr_proc);
        list_add(blocked_procs, curr_proc);
        list_add(my_cvar->wait, curr_proc);

        return SUCCESS;

}

Cvar *findCvar(int cvar_id)
{
        Node *temp = cvars->head;
        while (temp != NULL){
                if ( ((Cvar*)temp->data)->id == cvar_id)
                        break;
                temp = temp->next;
        }
        if(temp == NULL){
                TracePrintf(0, "Cvar does not exist\n");
                return ERROR;
        } 
        return (Cvar*) temp->data;
}

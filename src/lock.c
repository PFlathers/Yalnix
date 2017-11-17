#include "globals.h"
#include "syscalls.h"
#include "lock.h"
#include "list.h"
#include "kernel.h"


//Intializes Lock.
//Warning: sets lock to free and proc_id to 0, no one has claimed it.
//      int *lock_idp: a pointer to userland that we should write the lock
//              id to.
int kernel_LockInit(int *lock_idp)
{
        Lock *lock_to_init = (Lock*) malloc(sizeof(Lock));
        lock_to_init->id = available_lock_id;
        available_lock_id++;

        lock_to_init->claimed = 0;
        lock_to_init->proc_id = 0;
        lock_to_init->waiters = init_list();

        list_add(locks, lock_to_init); 
        *lock_idp = lock_to_init->id;

	return SUCCESS;
}

//Try to acquire the lock. 
//If the lock is free give the process the lock
//Else put the lock on a waiters list so we know
//priority.
//      lock_id: the id of the lock we are trying to acquire
int kernel_Acquire(int lock_id)
{
        TracePrintf(6, "Acquiring lock\n");

        Lock *my_lock = (Lock*) kernel_findLock(lock_id);
        //we didnt find the lock
        if( my_lock == NULL){
                TracePrintf(6, "lock does not exisit\n");
                return ERROR;

        //lock is already claimed. add curr proc to waiters. if its not already 
        //there.
        } 
        if (my_lock->claimed == 1){
                TracePrintf(6, "lock is already claimed\n");
                if (my_lock->proc_id == curr_proc->process_id){
                        TracePrintf(6, "\t I am the claimer\n");
                        return SUCCESS;
                }
                //
                // check if I'm on the waiters list
                Node *temp = my_lock->waiters->head;
                while (temp != NULL){
                        if ( (pcb*)(temp->data) == curr_proc ){
                                return SUCCESS;
                        }
                        temp = temp->next;
                }
                // add me to it and switch proc
                /* this is probably very wrond, but 
                i don't understand patrick's logit */
                list_add(my_lock->waiters, curr_proc);
                goto_next_process(curr_proc->user_context, 1);

                // when returned from the last process
                return  SUCCESS;
        } 
        else{
                my_lock->claimed = 1;
                my_lock->proc_id = curr_proc->process_id;
                return SUCCESS;
        }

	return ERROR;
}

// If the proces has the lock, release it.
//      lock_id: the id of the lock we are trying to acquire
int kernel_Release(int lock_id)
{
        TracePrintf(6, "Releasing lock\n");
        Node *temp = locks->head;
        Lock *my_lock = NULL;
        //Get the lock we are looking for
        while(temp != NULL){
                if ( ((Lock*)(temp->data))->id == lock_id){
                        my_lock= (Lock*) temp->data;
                        break;
                }
                temp = temp->next;
        }

        //we didnt find the lock
        if( my_lock == NULL){
                TracePrintf(6, "ERROR pid %d:lock does not exisit \n", curr_proc->process_id);
                return ERROR;
        }

        //Current process lied or is misinformed
        if (!my_lock->claimed || my_lock->proc_id != curr_proc->process_id){
                TracePrintf(6, "ERROR: pid %d:messing with lock \n", curr_proc->process_id);
                return ERROR;
        }


        if (list_count(my_lock->waiters) == 0){
                my_lock->claimed = 1;
                my_lock->proc_id = 0;
                return SUCCESS;
        }
        else {
                pcb *waiter_proc = list_pop(my_lock->waiters);
                my_lock->proc_id = waiter_proc->process_id;
                list_add(ready_procs, (void*) waiter_proc);
        }

        return SUCCESS;
}

#include "globals.h"
#include "syscalls.h"
#include "lock.h"
#include "kernel.h"


//Prototypes for lock.

//Intializes Lock.
//Warning: sets lock to free and proc_id to 0, no one has claimed it.
int LockInit(Lock *lock_to_init)
{
        lock_to_init = (Lock*) malloc(sizeof(Lock));
        lock_to_init->id = available_lock_id;
        available_lock_id++;

        lock_to_init->claimed = 0;
        lock_to_init->proc_id = 0;
        init_list(lock_to_init->waiters);

        list_add(locks, lock_to_init); 

	return 0;
}

int Acquire(int lock_id)
{
        TracePrintf(6, "Acquiring lock\n");
        Node *temp = locks->head;
        Lock *my_lock = NULL;
        //Get the lock we are looking for
        while(temp != NULL){
                if ( (Lock*)(temp->data)->id == lock_id){
                        my_lock= (Lock*) temp->data;
                        break;
                }
                temp = temp->next;
        }

        //we didnt find the lock
        if( my_lock == NULL){
                TracePrintf(6, "lock does not exisit\n");
                return ERROR;

        //lock is already claimed. add curr proc to waiters. if its not already 
        //there.
        } else if (my_lock->claimed == 1){
                TracePrintf(6, "lock is already claimed\n");
                temp = waiters->head;
                while (temp != NULL){
                        if ( (pcb*)(temp->data) == curr_proc ){
                                return SUCCESS;
                        }
                        temp = temp->next;
                }
                list_add(my_lock->waiters, curr_proc);
                return 
        } else{
                TracePrintf(6, "Lock aquired\n");
                my_lock->proc_id = curr_proc->process_id;
                my_lock->claimed = 1;
                list_remove(my_lock->waiters, curr_proc);
                return SUCCESS;
        }

	return ERROR;
}

int Release(int lock_id)
{
        TracePrintf(6, "Releasing lock\n");
        Node *temp = locks->head;
        Lock *my_lock = NULL;
        //Get the lock we are looking for
        while(temp != NULL){
                if ( (Lock*)(temp->data)->id == lock_id){
                        my_lock= (Lock*) temp->data;
                        break;
                }
                temp = temp->next;
        }

        //we didnt find the lock
        if( my_lock == NULL){
                TracePrintf(6, "lock does not exisit\n");
                return ERROR;
        // caller doesnt have the lock to release
        } else if (my_lock->proc_id != curr_proc->process_id){
                TracePrintf(6, "you do not have the lock\n");
                return ERROR;
        } else {
                Node *temp_from_ready = ready_procs->head;
                Node *temp_from_waiters = my_lock->waiters->head;
                
                while (temp_from_waiters != NULL){
                        temp_from_ready = ready_procs->head;
                        // Proactively moving the next in the waiting line to
                        // the front. So it gets the lock next;
                        while(temp_from_ready != NULL){
                                if ( (pcb*)(temp_from_ready->data) == (pcb*)(temp_from_waiters->data) ){
                                        next_pcb = (pcb *) temp_from_ready->data;
                                        list_remove(ready_procs, next_pcb);
                                        list_push(ready_procs, next_pcb);
                                        break;
                                }
                                temp_from_waiters = temp_from_waiters->next;
                        }

                }
                my_lock->claimed = 0;
                return SUCCESS;
        }
	return ERROR;
}

int LockDestroy(int lock_id)
{
	return 0;
}

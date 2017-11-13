#include "globals.h"
#include "syscalls.h"
#include "pipe.h"

#include "pcb.h"
#include "kernel.h"

/*------------------------------------------------- kernel_Fork -----
|  Function kernel_Fork
|
|  Purpose:  Fork is the only way new_processes are created in Yalnix;
|   		this is the only function that should have a call to new_process
| 			other than StartKernel
|
|  Parameters:
|      1) UserContext *user_context (IN/OUT) -- user context of currently
| 				running process - used to bootstrap the parent's nad child's
| 				UC to get them on the same page; may be updated at context
|				switching
|
|  Returns:  this is hte only function that returns _TWICE_. Separately
| 			as a kid (return value == 0) and as a parent (return value ==
| 			child's id). 
*-------------------------------------------------------------------*/
//TODO: Fix this.
int kernel_Fork(UserContext *user_context)
{
        TracePrintf(3, "kernel_Fork ### Start\n");

        pcb *child;
        pcb *parent; 

        int temp_pfn; // store the most recently poped pfn
        int dest_pg; //page in kernel for mappin a frame 

        int bcp_pfn;
        int bcp_valid;

        int retval, i;

        // current process becomes parent
        parent = curr_proc;
        memcpy((void *) parent->user_context, (void *) user_context, sizeof(UserContext));

        // bootstrap the new process for the child
        child = new_process(user_context);

        // init child's uc with the parent's uc
        memcpy((void *) child->user_context, (void *) parent->user_context);
        // but it needs new page tables
        child->region0_pt = (struct pte *) malloc( KERNEL_PAGE_COUNT * sizeof(struct pte) );
        ALLOC_CHECK(child->region0_pt, "kernel_Fork");
        child->region1_pt = (struct pte *) malloc(VREG_1_PAGE_COUNT * sizeof(struct pte));
        ALLOC_CHECK(child->region1_pt, "kernel_Fork");


        // child's heap base page and brk are the same as parrents
        child->heap_start_pg = parent->heap_start_pg;
        child->brk_address = parent->brk_address;
        TracePrintf(0, "BRK ADDRESSES IN FORK \nparent:%d\nchild:%d\n", parent->brk_address, child->brk_address);

        TracePrintf(6, "kernel_Fork: succesfully bootstraped new proces \n");

        TracePrintf(6, "kernel_Fork: starting pagetable copy\n");

        // copy parent's pgtbl into child
        memcpy((void *) child->region1_pt, (void *) parent->region1_pt, \
                VREG_1_PAGE_COUNT * sizeof(struct pte));

        memcpy( (void *) child->region0_pt, (void *) parent->region0_pt, \
                KERNEL_PAGE_COUNT * sizeof(struct pte) );

        TracePrintf(6, "kernel_Fork: end pagetable copy\n");


        TracePrintf(6, "kernel_Fork: create frames for pagetable\n");
        for (i = 0; i<KERNEL_PAGE_COUNT; i++){
                if (list_count(empty_frame_list) <= 0){
                        return ERROR;
                }

                temp_pfn = (int) list_pop(empty_frame_list);
                (*(child->region0_pt + i)).pfn = FRAME_TO_PAGE(temp_pfn);
        }


        for (i=0; i<VREG_1_PAGE_COUNT; i++){
                if ( (*(child->region1_pt + i)).valid == (u_long) 0x1 ) {

                        if (list_count(empty_frame_list) <= 0){
                                return ERROR;
                        }
                        temp_pfn = (int) list_pop(empty_frame_list);
                        (*(child->region1_pt + i)).pfn = FRAME_TO_PAGE(temp_pfn);

                }
                else{
                        // invalidate the frame
                        (*(child->region1_pt + i)).pfn = (u_long) 0x0;
                }
        }
        TracePrintf(6, "kernel_Fork: end creating frames for pagetable\n");


        TracePrintf(6, "kernel_Fork: copy parent's memory into child\n");
        dest_pg = (KERNEL_STACK_BASE >> PAGESHIFT) - 1;

        bcp_valid = r0_ptlist[dest_pg].valid;
        bcp_pfn = r0_ptlist[dest_pg].pfn;

        // destination page is valid
        r0_ptlist[dest_pg].valid = (u_long) 0x1;

        // prepare for memcopy
        unsigned int src, dst;
        dst = dest_pg << PAGESHIFT;

/* Per Sean's suggestions */
        for (i=0; i<KERNEL_PAGE_COUNT; i++){
                src = KERNEL_STACK_BASE + (i*PAGESIZE);

                r0_ptlist[dest_pg].pfn = (*(child->region0_pt + i)).pfn;
                WriteRegister(REG_TLB_FLUSH, TLB_FLUSH_0);


                memcpy((void*) dst, (void *) src, PAGESIZE);
        }


        TracePrintf(6, "\tCopied kernel frames\n");

        for (i=0; i<VREG_1_PAGE_COUNT; i++) {
                if ( (*(child->region1_pt + i)).valid == 0x1 ) {
                        src = VMEM_1_BASE + (i * PAGESIZE);

                        r0_ptlist[dest_pg].pfn = (*(child->region1_pt + i)).pfn;
                        WriteRegister(REG_TLB_FLUSH, TLB_FLUSH_0);


                        memcpy((void*) dst, (void *) src, PAGESIZE);
                }
        }

        TracePrintf(6, "\t Copied user frames \n");

        r0_ptlist[dest_pg].valid = bcp_valid;
        r0_ptlist[dest_pg].pfn = bcp_pfn;
        TracePrintf(6, "\t Restored frames from backup \n");

        WriteRegister(REG_TLB_FLUSH, TLB_FLUSH_0);
        TracePrintf(6, "kernel_Fork: end copy parent's memory into child\n");


        TracePrintf(6, "kernel_Fork: bookkeeping\n");
        TracePrintf(6, "\t: child->parent = parent\n");

        child->parent = parent;
        TracePrintf(6, "\t: parent->children = children\n");

        if (parent->children == NULL){
                parent->children = init_list();
                bzero(parent->children, sizeof(List));
        }


        list_add(parent->children, (void *) child);
        TracePrintf(6, "\t: add to child to allprocs and parent to ready procs \n");

        list_add(all_procs, (void*) child);
        list_add(ready_procs, (void *) parent);


        TracePrintf(6, "kernel_Fork: context switch arr\n");
        if (context_switch(parent, child, user_context) != 0) {
                return ERROR;
        }

        TracePrintf(6, "kernel_Fork: end context switch arr\n");


        TracePrintf(6, "kernel_Fork: return\n");
        if (curr_proc == parent){
                TracePrintf(6, "\t kernel_Fork; returning to parent  \n");
                TracePrintf(3, "kernel_Fork ### end\n");
                return child->process_id;

        }
        else if (curr_proc == child){
                TracePrintf(6, "\t kernel_Fork; returning to child  \n");
                TracePrintf(3, "kernel_Fork ### end\n");
                return 0;         
        }
        else{
                return ERROR;
        }

}

	

/* --------------------------kernel_Exec-----------------------------------
 * Loads a program into the current process and begins execution of that program.
 *
 * Function variables:
 *      uc: the current user context
 *      filename: the file that we have to execute
 *      argev; the arguements that we are going to pass to LoadProg so that we 
 *              can load the program and pass it the proper arguements.
 * Returns Sucess if we sucessfully exec the program. returns ERORR if there is
 * a problem with Load Prog. Common error is that the file doesnt exist...
 *------------------------------------------------------------------------*/
int kernel_Exec(UserContext *uc, char *filename, char **argvec)
{
	TracePrintf(3, "kernel_Exec ### start \n");
        TracePrintf(3, "%s" , filename);

	int i,retval;
	// cont the number of arguments
	int argc = 0;
	while (argvec[argc] != NULL){
		argc++;
	}
	/*change pcb and uc to look lke a blank process */ 
	pcb *proc = curr_proc;

	// user context
		// vector code and addess should be as is
		// registers is cleared (there is 8 of them)
	bzero((void *) uc->regs, (8*sizeof(u_long)) );
		// privileged regs are modified in Load Program


	// pcb 
		// id, uc as is
		// has_kc is set to 0 as we need to bootstap it
	proc->has_kc = 0;
		// heap and brk ase set in the Load Program 
		// region0_pt trashed and reinintialized
	int trash_pfn;
	int assign_pfn;
	for (i = 0; i < KERNEL_PAGE_COUNT; i++){
		// put old fame back on the empty list
		trash_pfn = (( (*(proc->region0_pt + i)).pfn << PAGESHIFT) / PAGESIZE);
		list_add(empty_frame_list, (void *) trash_pfn);
		// pop one and assign
		assign_pfn = ((u_long) (((int)(list_pop(empty_frame_list)) * PAGESIZE) >> PAGESHIFT));
		(*(proc->region0_pt + i)).pfn = assign_pfn;
	}
	WriteRegister(REG_TLB_FLUSH, TLB_FLUSH_0);
		// region1_pt trashed and reinintialized

	for (i = 0; i < VREG_1_PAGE_COUNT; i++) {
		if ( (*(proc->region1_pt + i)).valid == 0x1 ){
			// thrash the page
			trash_pfn = (( (*(proc->region1_pt + i)).pfn << PAGESHIFT) / PAGESIZE);
			list_add(empty_frame_list, (void *) trash_pfn);
			
			// reset validity nad pfn to default
			(*(proc->region1_pt + i)).pfn = (u_long) 0x0;
			(*(proc->region1_pt + i)).valid = (u_long) 0x0;

		}
	}

	WriteRegister(REG_TLB_FLUSH, TLB_FLUSH_1);


        TracePrintf(3, "kexev: %s\n", argvec[0]);
        TracePrintf(3, "filename: %s\n", filename);
        TracePrintf(3, "proc: %d\n", proc->process_id);
        TracePrintf(3, "kexev: %s\n", argvec[0]);
        for (i = 0; i < argc; i ++){
                TracePrintf(3, "%s\n", argvec[i]);
        }

	/* load next program */

	retval = LoadProgram(filename, argvec, proc);
	if (retval != SUCCESS) {
		TracePrintf(3, "kernel_Exec: could not load program \n");
		exit(ERROR);
	}

	/* replicate the rest of the uc */ 
	memcpy((void *) uc, (void *) proc->user_context, sizeof(UserContext));

	/* return */
	return SUCCESS;

}


/*-------------------------Kernel_Exit----------------------------------------
 * Function: kernel_Exit
 * Purpose: implemtes the exit syscall. for details please see syscalls.h
 *
 * (1) if we have kids, mark them as orphans.
 * (2) free all resources associated with the process and mark its return
 * status.
 * (3) if this is the root process, halt the system.
 *
 * Parameters:
 *	status: the exit status of the process.
 *	uc: the current user context.
 *-------------------------------------------------------------------------*/
// TODO: Figure out if we need to free the kernel context and/or user context
// of the pcb.
void kernel_Exit(int status, UserContext *uc)
{

        TracePrintf(3, "kernel_exit ### start\n");
        Node *temp;
        pcb *p;
        pcb *child;
        pcb *exiting_p = curr_proc;

        int orphan_flag = (curr_proc->parent == NULL);
        //int orphan_flag = (curr_proc->parent == NULL ? 1 : 0);
        if(!orphan_flag){
                list_remove(blocked_procs, curr_proc->parent);
                list_add(ready_procs, curr_proc->parent);
        }

        if (curr_proc->process_id == 0){
            Halt();
        }

        child = NULL;

        // has kids
        if (curr_proc->children != NULL && list_count(curr_proc->children) >0){
            while ( (p = (pcb *) list_pop(curr_proc->children)) != NULL){
                p->parent = NULL;
            }
        }
        TracePrintf(6, "\t: passed has kids\n");

        if (curr_proc->zombiez != NULL && list_count(curr_proc->zombiez) > 0){
            while ( (p = (pcb *) list_pop(curr_proc->zombiez)) != NULL){
                Node *temp = zombie_procs->head;
                pcb *dead_status = NULL;
                while(temp != NULL){
                    if ( ((pcb*)(temp->data))->process_id == curr_proc->process_id){
                            dead_status= (pcb*) temp->data;
                            break;
                    }
                    temp = temp->next;
                 }

        //we didnt find the lock
                if( dead_status == NULL){
                    TracePrintf(6, "no exiting thild\n");
                }
                else {
                    list_remove(zombie_procs, dead_status);
                    free_pagetables(dead_status);
                }
            }
        }
        TracePrintf(6, "\t: passed has zombiez\n");
        
        //Create a zombie process
        //
        /*
        z_pcb *zp = (z_pcb*) malloc(sizeof(z_pcb));
        zp->exit_status = status;
        zp->parent = curr_proc->parent;
        zp->process_id = curr_proc->process_id;
        */

        if (curr_proc->parent != NULL){
            p = curr_proc->parent;

            if (list_remove(p->children, (void*) curr_proc) != 0){
                TracePrintf(6, "can't remove curr from parent thild\n");
            }

            if (p->zombiez == NULL){
                p->zombiez = init_list();
                bzero(p->zombiez, sizeof(List));
            }
            list_add(p->zombiez, (void*) curr_proc);
        }

        TracePrintf(6, "\t: passed has parent\n");
        if (list_remove(all_procs, curr_proc) != 0){
            TracePrintf(6, "can't remove curr from allprocs\n");
        }

        TracePrintf(6, "\t: well, shit\n");



        list_add(zombie_procs, (void *) curr_proc);

        if(orphan_flag){
                // trash pt
                free_pagetables(curr_proc);
                TracePrintf(6, "\t: freed pagetables\n");
        }
        pcb *next = list_pop(ready_procs);
        if (!next){
            TracePrintf(6, "\t: we are screwed\n");
        }
        TracePrintf(6, "\t: popped ready\n");

        // while (next->has_kc == 0){
        //     list_add(ready_procs, next);
        //     next = list_pop(ready_procs);
        // }
        if (next->has_kc == 0){
            TracePrintf(6, "\t: we are in the new one - shit\n");
            list_add(ready_procs, next);
            next = list_pop(ready_procs);
        }
        TracePrintf(6, "\t: now next kc is %d\n", next->has_kc);
        TracePrintf(6, "\t: passed safety check\n");
       // cycle_process(uc);
        if (context_switch((pcb *) NULL, next, uc) != SUCCESS ){
            exit(ERROR);
        }
    }


//         if(exiting_p->process_id == 0)
//                 Halt();
       
//         list_remove(ready_procs, (void*) exiting_p);
//         list_add(zombie_procs, (void*) exiting_p);
//         exiting_p->exit_status= status;
       
//         //parent is still alive 
//         if(exiting_p->parent != NULL){
//                 //tell parent you died
//                 TracePrintf(0, "parent:%u\n", exiting_p->parent->process_id);
//                 if(exiting_p->parent->zombiez == NULL)
//                        exiting_p->parent->zombiez = init_list();

// 		// add to parent's zombie list
//                 list_add(exiting_p->parent->zombiez, (void*) exiting_p); 
// 		TracePrintf(0, "I just added %u to my parents zombie list\n",
// 				exiting_p->process_id);
// 		// no longer a child cause you died. 
//                 list_remove(exiting_p->parent->children, (void*) exiting_p);
//                 TracePrintf(0, "parent's zombies: %u\n", ((pcb*)exiting_p->parent->zombiez->head->data)->process_id);
// 		TracePrintf(0, "my parent has %u zombies\n",
// 				exiting_p->parent->zombiez->count);

//         //you are an orphan :(
//         } else {
// 		orphan_flag = 1;
//         }

//         //inform your children of your demise. 
//         if(exiting_p->children != NULL && list_count(exiting_p->children) > 0){
// 		TracePrintf(6, "Informing children I am dead\n");
//                 //Mark children as orphans
//                if( list_count(exiting_p->children) > 0){
//                         temp = exiting_p->children->head;
//                         while(temp != NULL){
//                                 ((pcb*) temp->data)->parent = NULL;
//                                 temp = temp->next;
//                         }
//                }
//                free(exiting_p->children);
//         }

//         //free your dead children
//         if(exiting_p->zombiez != NULL && list_count(exiting_p->zombiez) > 0){
//                 temp = exiting_p->zombiez->head;
//                 while(temp != NULL){
//                         p = (pcb*) temp->data;
//                         list_remove(all_procs, (void*)p);
// //!!!!process thinks that its parent is a child/zombie and therefore is trying to kill
// // free it. This is a fork problem.
// //Fixed for now.
// 			if (p->process_id == exiting_p->process_id)
// 			       TracePrintf(0, "I think I'm a zombie :p\n");
// 			else if (p->process_id == exiting_p->parent->process_id)
// 				TracePrintf(0, "I think my parent, %u, is my zombie :p\n", p->process_id);
// 			else{
// 				free_pagetables(p);  
// 				free(p);
// 			}
//                         temp = temp->next;
//                 }
//         }
	
// 	if(orphan_flag){
// 		TracePrintf(6, "I'm an orphan and I'm killing myself\n");
// 		list_remove(all_procs, exiting_p);
// 		list_remove(ready_procs, exiting_p);
// 		list_remove(blocked_procs, exiting_p);
// 		list_remove(zombie_procs, exiting_p);
// 		free_pagetables(exiting_p);
// 		free(p);
// 	}
// 	TracePrintf(0, "KERNEL EXIT ### END\n");
//         cycle_process(uc);

// }
// void cycle_process(UserContext *uc)
// {
// 	if (list_count(ready_procs) < 1) {
// 		TracePrintf(3, "kernel_Exit: no items on the ready queue - exiting\n");
// 		exit(ERROR);
// 	}
// 	else {
// 		if (goto_next_process(uc, 0) != SUCCESS) {
// 			TracePrintf(3, "kernel_Exit: failed to context switch - exiting\n");
// 			exit(ERROR);
// 		}
//                 TracePrintf(0, "Sucess");
// 	}
// }
/*
 * Helper function that frees the pagetables of a pcb by returning them to 
 * empty_frame_list for use in another process later.
 *
 * Function variables:
 *      myproc: the process that we want to recycle to pagetables.
 */



void free_pagetables(pcb* myproc)
{
        int i;
        int trash_pfn;

         
        //Recycle the kernel page tables
        for (i = 0; i < KERNEL_PAGE_COUNT; i ++){
                if( (*(myproc->region0_pt + i)).valid == 0x1){
                        trash_pfn = FRAME_TO_PAGE( (*(myproc->region0_pt + i)).pfn );
                        list_add(empty_frame_list, (void *) trash_pfn);

                        (*(myproc->region0_pt + i)).valid = (u_long) 0x0;
                        (*(myproc->region0_pt + i)).pfn = (u_long) 0x0;
                }
        }
        
        //Recycle the userland page tables
        for (i = 0; i < VREG_1_PAGE_COUNT; i++){
                if( (*(myproc->region1_pt + i)).valid == 0x1 ){
                        trash_pfn = FRAME_TO_PAGE( (*(myproc->region0_pt + i)).pfn );
                        list_add(empty_frame_list, (void*) trash_pfn);

                        (*(myproc->region1_pt + i)).pfn = (u_long) 0x0;
                        (*(myproc->region1_pt + i)).valid= (u_long) 0x0;
                }
        }
        
        free(myproc->user_context);
        free(myproc->kernel_context);

        //Dont forget to flush.
        WriteRegister(REG_TLB_FLUSH, TLB_FLUSH_ALL);
}


/*------------------------------------------------- Kernel_Wait -----
 |  Function kernel_Wait
 |
 |  Purpose:  implements the wait syscall for details please see syscalls.h
 |			  it consists of 3 main parts:
 |          1) if we have kids that exited return their info 
 | 			2) if we have blocking kids set up a block
 |			3) place where wait returns implements pretty much 1) again
 |
 |  Parameters:
 |      status_ptr (OUT or IN/OUT): exit status of the returned chiled 
 |									copied to the integer 
 |                   
 |
 |  Returns:  int with returing childs PID
 *-------------------------------------------------------------------*/
int kernel_Wait(int * status_ptr, UserContext *uc)
{
	TracePrintf(3, "kernel_Wait ### start \n");
	int child_pid_retval;
    pcb *found_item = NULL;
    Node *temp;
	
	// check if process has children
	// if not, return error as the user was dumb enough to call wait wihtout them
	pcb *parent = curr_proc;
	TracePrintf(6, "kernel_Wait: curr waiting id: %d\n", curr_proc->process_id);

	if (parent->children == NULL){
		TracePrintf(3, "kernel_Wait (ln31): %d called Wait without children initialized\n", parent->process_id);
		return(ERROR);
	}


	// check if process has children that exited
	if (parent->zombiez == NULL){
		if (list_count(parent->children) <= 0 ) {
			TracePrintf(3, "kernel_Wait (ln40): %d called Wait without active children, and with no zombie children initialized\n", parent->process_id);
			return(ERROR);
		}
	}
	else if (list_count(parent->children) <= 0 && list_count(parent->zombiez) <= 0) {
		TracePrintf(3, "kernel_Wait (ln46): %d called Wait without active or zombie children\n", parent->process_id);
		return(ERROR);
	}

	TracePrintf(6, "kernel_Wait: done with cheking\n");
	/* Actual work under */

	// if has dead children, find them and remove them from the 
	// parent->zombie list and global zombies list
	if (parent->zombiez != NULL && list_count(parent->zombiez) > 0) {
        	TracePrintf(6, "kernel_Wait: found zombie children\n");
        	// pop from parent list
        	pcb *child_pcb = (pcb*) list_pop(parent->zombiez);
        	child_pid_retval = child_pcb->process_id;

    		TracePrintf(6, "kernel_Wait: I should remove procces: %d from zombie list \n", child_pcb->process_id);
            temp = zombie_procs->head;
            found_item = NULL;
            //Get the lock we are looking for
            while(temp != NULL){
                if ( ((pcb*)(temp->data))->process_id == child_pid_retval){
                    found_item = (pcb*) temp->data;
                    break;
                }

                temp = temp->next;
            }

            if (found_item == NULL) {
                    TracePrintf(3, "wait: process not found in zombiez\n");
                    return(ERROR);
            }
            // if my understanding is correct, status should be 
    		// the pointer to the process
    		*status_ptr = *((int *) child_pcb);
    		// remove from global list (found in kernel.h)
    		list_remove(zombie_procs, child_pcb);


    		// return the pid of the child that returned (mind == blown)
    		TracePrintf(3, "kernel_Wait ### end \n");
    		return child_pid_retval; 
	}
	
	TracePrintf(6, "kernel_Wait: parent's pid %d", parent->process_id);
	// if we have live kids waiting, 
	// modify the same block structure we used for delay
	parent->block->is_active = ACTIVE;
	parent->block->type = WAIT;
	// data will point to the blocking process (in this case parent)
	parent->block->data = (void *) parent;
	 
	/* book keeping */
	list_add(blocked_procs, (void *) parent);

	// switch to other process if needed
	if (list_count(ready_procs) <= 0) {
		TracePrintf(3, "kernel_Wait: no items on the ready queue - exiting\n");
		exit(ERROR);
	}
	else {
		if (goto_next_process(uc, 0) != SUCCESS) {
			TracePrintf(3, "kernel_Wait: failed to kontext switch - exiting\n");
			return ERROR;
		}
	}

	// pc should be pointing here after the wait
            TracePrintf(3, "kernel_Wait: found a child after blocking\n");
	// at thois point, the only scenario is that we are returning from being
	// blocked so her we removing exited child and removing it 
	if (list_count(parent->zombiez) < 1) {
		TracePrintf(3, "kernel_Wait: returned after unknown block - exiting\n");
		return(ERROR);
	}

	// remove exited child from the pcb list
	pcb *child_pcb = (pcb *) list_pop(parent->zombiez);
	child_pid_retval = child_pcb->process_id;

            temp = zombie_procs->head;
    
    //Get the lock we are looking for
    while(temp != NULL){
        if ( ((pcb*)(temp->data))->process_id == child_pid_retval){
            found_item = (pcb*) temp->data;
            break;
        }

        temp = temp->next;
    }

    if (found_item == NULL) {
            TracePrintf(3, "wait: process not found in zombiez\n");
            return(ERROR);
    }  
	
	// remove exited child from the global list
	*status_ptr = *((int *) child_pcb);
	list_remove(zombie_procs, child_pcb);
        free_pagetables(child_pcb);
	

	// return the pid of the child that returned (mind == blown)
	TracePrintf(3, "kernel_Wait ### end \n");
	return child_pid_retval; 
}


int kernel_GetPid()
{
	return curr_proc->process_id;
}

int kernel_Brk(void *addr)
{

	int i; // counter
	int page_id; // temp

	// bottom of the heap is marked in pcb
	unsigned int heap_bottom = curr_proc->heap_start_pg;
	// round the address and subtract the total page count
	unsigned int heap_top = (UP_TO_PAGE(addr) >> PAGESHIFT) - VREG_0_PAGE_COUNT;
	// reverse for the stack bottom (close to heap top)
	unsigned int stack_bottom = (DOWN_TO_PAGE(curr_proc->user_context->sp) >> PAGESHIFT) - VREG_0_PAGE_COUNT;
	// stack top is limit - count
	unsigned int stack_top = ((VMEM_1_LIMIT >> PAGESHIFT) - VREG_1_PAGE_COUNT);


	// check the addresses
	unsigned int u_addr = (unsigned int) addr; 
        TracePrintf(3, "kernel_Brk: current brk = %u, requesting: %u \n", curr_proc->brk_address, u_addr);

        unsigned int stack_bottom_page = (unsigned int) (stack_bottom << PAGESHIFT);
        unsigned int heap_top_page = (unsigned int) (heap_bottom << PAGESHIFT);
	if (u_addr > stack_bottom_page || 
		u_addr < heap_top_page ){
		TracePrintf(1, "kernel_Brk: address requested (uaddr = %u) out of bounds %u to %u \n", u_addr, heap_top_page, stack_bottom_page);
		return ERROR;
	}

	// heap
	for (i = heap_bottom; i<= heap_top; i++){
		if ((*(curr_proc->region1_pt + i)).valid != 0x1) {
			// check for empty space
                        if (list_count(empty_frame_list) < 1){
				TracePrintf(2, "kernel_Brk: out of memory");
				return ERROR;
			}

			// all heap pages are valid, rw, and theis phisical number is based on page id
			(*(curr_proc->region1_pt + i)).valid = (u_long) 0x1;
			(*(curr_proc->region1_pt + i)).prot = (u_long) (PROT_READ | PROT_WRITE);
			page_id = (int) list_pop(empty_frame_list);
			(*(curr_proc->region1_pt + i)).pfn = (u_long) ( (PMEM_BASE + 
                  (page_id * PAGESIZE) ) >> PAGESHIFT);

		}
	} // end heap traversal

	// heap to stack
	for ( i = heap_top; i < stack_bottom; i++){
		if ((*(curr_proc->region1_pt + i)).valid == 0x1) {
			(*(curr_proc->region1_pt + i)).valid = (u_long) 0x0;
			unsigned int idx = ((*(curr_proc->region1_pt + i)).pfn << PAGESHIFT) / PAGESIZE;
			list_add(empty_frame_list, (void *) idx);
		}
	}

	curr_proc->brk_address = heap_top << PAGESHIFT;
	return SUCCESS;
}

int kernel_Delay(UserContext *user_context, int clock_ticks)
{
	TracePrintf(3, "kernel_Delay ### start ");
	if (clock_ticks == 0)
		return SUCCESS;
	if (clock_ticks < 0)
		return ERROR;
	

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
	void *temp;
	
    if ((temp = fintCvar(id)) != NULL ){
        Cvar *c = (Cvar *) temp;
        if (list_count(c->waiters) > 0){
            TracePrintf(6, "Can't release cvar: waiters alive \n");
            return ERROR;
        }
        list_remove_pcb(cvars, c);
        free(c);
    }
    else if ((temp = fintLock(id)) != NULL ){
        Lock *l = (Lock *) temp;
        if (l->claimed){
            TracePrintf(6, "Can't release lock: taken \n");
            return ERROR;
        }
        if (list_count(l->waiters) > 0){
            TracePrintf(6, "Can't release lock: waiters \n");
            return ERROR;
        }

        list_remove_pcb(locks, l);
        free(l);
    }
    else if( (temp = fintPipe(id)) != NULL ){
        Pipe *p =  (Pipe*) temp;
        if (list_count(p->pipe_queue) > 0){
            TracePrintf(6, "Can't release pipe: waiters (i.e.) \n");
            return ERROR;
        }

        list_remove_pcb(pipes, p);
        free(p->buffer);
        free(p);

    }
	return 0;
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
                return NULL;
        } 
        return (Cvar*) temp->data;
}
Lock *findLock(int lock_id)
{
        Node *temp = locks->head;
        while (temp != NULL){
                if ( ((Lock*)temp->data)->id == lock_id)
                        break;
                temp = temp->next;
        }
        if(temp == NULL){
                TracePrintf(0, "Lock does not exist\n");
                return NULL;
        } 
        return (Lock*) temp->data;
}
Pipe *findPipe(int lock_id)
{
        Node *temp = pipes->head;
        while (temp != NULL){
                if ( ((Pipe*)temp->data)->id == lock_id)
                        break;
                temp = temp->next;
        }
        if(temp == NULL){
                TracePrintf(0, "Pipe does not exist\n");
                return NULL;
        } 
        return (Pipe*) temp->data;
}

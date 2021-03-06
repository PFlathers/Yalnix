#include "globals.h"
#include "syscalls.h"
#include "pipe.h"
#include "cvar.h"
#include "pcb.h"
#include "kernel.h"
#include "kernel_utils.h"

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

        TracePrintf(6, "kernel_Fork: \t succesfully bootstraped new proces \n");

        TracePrintf(6, "kernel_Fork: \t starting pagetable copy\n");

        // copy parent's pgtbl into child
        memcpy((void *) child->region1_pt, (void *) parent->region1_pt, \
                VREG_1_PAGE_COUNT * sizeof(struct pte));

        memcpy( (void *) child->region0_pt, (void *) parent->region0_pt, \
                KERNEL_PAGE_COUNT * sizeof(struct pte) );

        TracePrintf(6, "kernel_Fork: \t end pagetable copy\n");


        TracePrintf(6, "kernel_Fork: \t create frames for pagetable\n");
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
        TracePrintf(6, "kernel_Fork: \t end creating frames for pagetable\n");


        TracePrintf(6, "kernel_Fork: \t copy parent's memory into child\n");
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


        TracePrintf(6, "kernel_Fork: \t copied kernel frames\n");

        for (i=0; i<VREG_1_PAGE_COUNT; i++) {
                if ( (*(child->region1_pt + i)).valid == 0x1 ) {
                        src = VMEM_1_BASE + (i * PAGESIZE);

                        r0_ptlist[dest_pg].pfn = (*(child->region1_pt + i)).pfn;
                        WriteRegister(REG_TLB_FLUSH, TLB_FLUSH_0);


                        memcpy((void*) dst, (void *) src, PAGESIZE);
                }
        }

        TracePrintf(6, "kernel_Fork:\t Copied user frames \n");

        r0_ptlist[dest_pg].valid = bcp_valid;
        r0_ptlist[dest_pg].pfn = bcp_pfn;
        TracePrintf(6, "kernel_Fork:\t restored frames from backup \n");

        WriteRegister(REG_TLB_FLUSH, TLB_FLUSH_0);
        TracePrintf(6, "kernel_Fork: \t end copy parent's memory into child\n");


        TracePrintf(6, "kernel_Fork: \t bookkeeping\n");
        TracePrintf(6, "kernel_Fork:\t: child->parent = parent\n");

        child->parent = parent;
        TracePrintf(6, "kernel_Fork:\t: parent->children = children\n");

        if (parent->children == NULL){
                parent->children = init_list();
                bzero(parent->children, sizeof(List));
        }


        list_add(parent->children, (void *) child);
        TracePrintf(6, "kernel_Fork:\t add to child to allprocs and parent to ready procs \n");

        list_add(all_procs, (void*) child);
        list_add(ready_procs, (void *) parent);


        TracePrintf(6, "kernel_Fork: \t context switch arr\n");
        if (context_switch(parent, child, user_context) != 0) {
                return ERROR;
        }

        TracePrintf(6, "kernel_Fork: \t end context switch arr\n");


        TracePrintf(3, "kernel_Fork: return\n");
        if (curr_proc == parent){
                TracePrintf(6, "\t kernel_Fork; returning to parent  \n");
                TracePrintf(3, "kernel_Fork parent ### end\n");
                return child->process_id;

        }
        else if (curr_proc == child){
                TracePrintf(6, "\t kernel_Fork; returning to child  \n");
                TracePrintf(3, "kernel_Fork child ### end\n");
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


        TracePrintf(6, "kexev: %s\n", argvec[0]);
        TracePrintf(6, "filename: %s\n", filename);
        TracePrintf(6, "proc: %d\n", proc->process_id);
        TracePrintf(6, "kexev: %s\n", argvec[0]);
        for (i = 0; i < argc; i ++){
                TracePrintf(3, "%s\n", argvec[i]);
        }

	/* load next program */

	retval = LoadProgram(filename, argvec, proc);
	if (retval != SUCCESS) {
		TracePrintf(3, "kernel_Exec: could not load program; pid %d\n", curr_proc->process_id);
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
        if(curr_proc->parent != NULL)
                TracePrintf(10, "parent's pid is %d\n", curr_proc->parent->process_id);
        Node *temp;
        pcb *p;
        pcb *child;
        pcb *exiting_p = curr_proc;

        // if it's on the ready list remove it
        temp = ready_procs->head;
        while(temp->next != NULL){
            if ( ((pcb*)temp->data)->process_id == exiting_p->process_id){
                list_remove(ready_procs, exiting_p);
                TracePrintf(6,"\t:exiting being removed from ready\n");

            }
            temp = temp->next;
        }
        TracePrintf(6,"\texiting process not on the list or \n");

        int orphan_flag = (exiting_p->parent == NULL || exiting_p->parent->process_id == 0);
        //int orphan_flag = (curr_proc->parent == NULL ? 1 : 0);
        if(!orphan_flag){
                list_remove(blocked_procs, exiting_p->parent);
                list_add(ready_procs, exiting_p->parent);
        }

        // if it's init
        if (exiting_p->process_id == 0){
            Halt();
        }

        child = NULL;

        // has kids
        if (exiting_p->children != NULL && list_count(exiting_p->children) > 0){
            while ( (p = (pcb *) list_pop(exiting_p->children)) != NULL){
                p->parent = NULL;
            }

            if (exiting_p->children->count == 0){
                TracePrintf(6, "\t :sfreeing %d children list", exiting_p->process_id);
                free(exiting_p->children);
            }
            else {
                TracePrintf(6, "\t :shit 1");
            }
        }
        TracePrintf(6, "\t: passed has kids\n");

        // has exited kids
        if (exiting_p->zombiez != NULL && list_count(exiting_p->zombiez) > 0){

            while ( (p = (pcb *) list_pop(exiting_p->zombiez)) != NULL){
               TracePrintf(10, "Removing a zombie\n"); 
                int child_pid = p->process_id;
                Node *temp = zombie_procs->head;
                pcb *dead_status = NULL;
                while(temp != NULL){

                    if ( ((pcb*)(temp->data))->process_id == child_pid){
                            dead_status= (pcb*) temp->data;
                            break;
                    }
                    temp = temp->next;
                 }
                
                list_remove(zombie_procs, dead_status);
                // free_pagetables(dead_status);

        //we didnt find the lock
        /*
                if( dead_status == NULL){
                    TracePrintf(6, "no exiting thild\n");
                }
                else {
                    list_remove(zombie_procs, dead_status);
                    free_pagetables(dead_status);
                }*/
            }
            if (exiting_p->zombiez->count == 0){
                TracePrintf(6, "\t :sfreeing %d zombie list", exiting_p->process_id);
                free(exiting_p->zombiez);
            } 
            else {
                TracePrintf(6, "\t : shit^2");
            }
        }
        TracePrintf(6, "\t: passed has zombiez\n");
        
        if (exiting_p->parent != NULL && exiting_p->parent->process_id != 0){
            p = exiting_p->parent;

            if (list_remove(p->children, (void*) exiting_p) != 0){
                TracePrintf(6, "kernelExit in (%d)\t can't remove curr from parent thild\n", curr_proc->process_id);
            }

            if (p->zombiez == NULL){
                p->zombiez = init_list();
                //bzero(p->zombiez, sizeof(List));
            }
            TracePrintf(10, "adding a zombie to my parent\n");
            list_add(p->zombiez, (void*) exiting_p);
        }

        TracePrintf(6, "\t: passed has parent\n");
        if (list_remove(all_procs, exiting_p) != 0){
            TracePrintf(6, "can't remove curr (%d) from allprocs\n", curr_proc->process_id);
        }

        TracePrintf(6, "\t: well, shit\n");



        if(orphan_flag){
                // trash pt
                free_pagetables(exiting_p);
                TracePrintf(6, "\t: freed pagetables\n");
        }else {
                list_add(zombie_procs, (void *) exiting_p);
        }
        pcb *next = list_pop(ready_procs);
        if (!next){
            TracePrintf(6, "\t: we are screwed\n");
        }
        TracePrintf(6, "\t: popped ready\n");

        // don't start from exit, that would be bed given our 
        // KCS implementation
        while (next->has_kc == 0){
            TracePrintf(6, "\t: we are in the new one - shit\n");
            list_add(ready_procs, next);
            next = list_pop(ready_procs);
        }
        TracePrintf(6, "\t: now next kc is %d\n", next->has_kc);
        
        TracePrintf(6, "\t: passed safety check\n");
        if (context_switch((pcb *) NULL, next, uc) != SUCCESS ){
            exit(ERROR);
        }
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



/*------------------------------------------------- kernel_GetPid -----
 |  Function kernel_GetPid
 |
 | Description: returns curr proc PID
 |
 |  Parameters:
 |      none
 |
 |  Returns:  int with current PID
 *-------------------------------------------------------------------*/
int kernel_GetPid()
{
	return curr_proc->process_id;
}




/*------------------------------------------------- kernel_Brk -----
 |  Function kernel_Brk
 |
 | Purpose:  sets the operating system’s idea of the lowest location 
 | not used by the program (called the “break”) to addr (rounded up to 
 | the next multiple of PAGESIZE bytes).
 |      1. check if addr in range
 |      2. allocate frames  and assign them to the process PTs
 |      3. traverse heap and make sure that permissions align
 |
 |  Parameters:
 |      void *addr (IN): address to set the brk to
 |                   
 |
 |  Returns:  SUCCESS if ok, ERROR otherwise
 *-------------------------------------------------------------------*/
int kernel_Brk(void *addr)
{

    TracePrintf(3, "kernel_Brk ### start \n");
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
		TracePrintf(3, "kernel_Brk (pid %d): address requested (uaddr = %u) out of bounds %u to %u \n", curr_proc->process_id, u_addr, heap_top_page, stack_bottom_page);
		return ERROR;
	}

    if ( ((unsigned int)addr >> PAGESHIFT) >= (DOWN_TO_PAGE(curr_proc->user_context->sp)>>PAGESHIFT) ){
        TracePrintf(3, "kernel_Brk (pid %d): address requested (uaddr = %u) out of bounds %u to %u \n", curr_proc->process_id, u_addr, heap_top_page, stack_bottom_page);
        return ERROR;
    }

	// heap
	for (i = heap_bottom; i<= heap_top; i++){
		if ((*(curr_proc->region1_pt + i)).valid != 0x1) {
			// check for empty space
                        if (list_count(empty_frame_list) < 1){
				TracePrintf(3, "kernel_Brk (pid %d): out of memory", curr_proc->process_id);
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

	curr_proc->brk_address = (int) addr;
    TracePrintf(3, "kernel_Brk ### end \n");
	return SUCCESS;
}



/*------------------------------------------------- kernel_Delay -----
 |  Function kernel_Delay
 |
 | Description:  The calling process is blocked until clock ticks clock 
 | interrupts have occurred after the call. 
 |      1. Set up block
 |      2. bookkeep
 |      3. Switch to new process
 |
 |  Parameters:
 |      UserContext *user_context (IN): current user context
 |      int clock_ticks (IN): how long to delay a process
 |                   
 |
 |  Returns:  SUCCESS if delay completed, ERROR otherwise
 *-------------------------------------------------------------------*/
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
	//list_remove(ready_procs, (void *) curr_proc);
	list_add(blocked_procs, (void *) curr_proc);

	if (list_count(ready_procs) < 1){
		TracePrintf(2, "no items to switch to! ");
		exit(FAILURE);
	} else {
		if (goto_next_process(user_context, 0) != SUCCESS){
			TracePrintf(3, "Delay (pid %d): goto_next_process failed with error", curr_proc->process_id);
		}
	}

	return SUCCESS;
}



/*------------------------------------------------- kernel_Reclaim -----
 |  Function kernel_Reclaim
 |
 | Purpose: Destroy the lock, condition variable, or pipe indentified by 
 | id. 
 | `    1. check if it's cvar/lock/pipe/buff
 |      2. check if it is in use
 }      3. follow procedure for reclaiming based on what we are reclaiming
 |
 |  Parameters:
 |      int id (IN): ID of the resource to be reclaimed
 |                   
 |
 |  Returns:  SUCCESS if reclaimed, ERROR otherwise
 *-------------------------------------------------------------------*/
int kernel_Reclaim(int id)
{
	void *temp;
	
    if ((temp = ((void*) kernel_findCvar(id)) ) != NULL ){
        Cvar *c = (Cvar *) temp;
        if (list_count(c->waiters) > 0){
            TracePrintf(6, "Proc %d Can't release cvar: waiters alive \n", curr_proc->process_id);
            return ERROR;
        }
        list_remove(cvars, c);
        while (list_pop(c->waiters) != NULL);
        free(c->waiters);
        free(c);
    }
    else if ((temp = ((void*) kernel_findLock(id)) ) != NULL ){
        Lock *l = (Lock *) temp;
        if (l->claimed){
            TracePrintf(6, "Proc %d Can't release lock: taken \n", curr_proc->process_id);
            return ERROR;
        }
        if (list_count(l->waiters) > 0){
            TracePrintf(6, "Proc %d Can't release lock: waiters \n", curr_proc->process_id);
            return ERROR;
        }

        list_remove(locks, l);
        while (list_pop(l->waiters) != NULL);
        free(l->waiters);
        free(l);
    }
    else if( (temp = ((void*) kernel_findPipe(id)) ) != NULL ){
        Pipe *p =  (Pipe*) temp;
        if (list_count(p->pipe_queue) > 0){
            TracePrintf(6, "Proc %d Can't release pipe: waiters (i.e.) \n", curr_proc->process_id);
            return ERROR;
        }

        list_remove(pipes, p);
        while (list_pop(p->pipe_queue) != NULL);
        free(p->pipe_queue);
        free(p->buffer);
        free(p);

    }
	return 0;
}


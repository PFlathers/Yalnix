#include "globals.h"
#include "syscalls.h"
#include "pipe.h"

#include "pcb.h"
#include "kernel.h"


int kernel_Fork()
{
	return 0;
}

int kernel_Exec(char *filename, char **argvec)
{
	return 0;
}

void kernel_Exit(int status)
{
}

int kernel_Wait(int * status_ptr)
{
	TracePrintf(3, "kernel_Wait ### start \n");
	int child_pid_retval;
	
	// check if process has children
	// if not, return error as the user was dumb enough to call wait wihtout them
	pcb *parent = curr_proc;
	TracePrintf(6, "kernel_Wait: curr waiting id: %d\n", curr_proc->process_id);
	if (parent->children == NULL){
		TracePrintf(3, "kernel_Wait (ln31): %d called Wait without children initialized\n", parent->process_id);
		return(ERROR);
	}

	// check if process has children that exited
	if (parent->zombies == NULL){
		if (list_count(parent->children) <= 0 ) {
			TracePrintf(3, "kernel_Wait (ln40): %d called Wait without active children, and with no zombie children initialized\n", parent->process_id);
			return(ERROR);
		}
	}

	if (list_count(parent->zombies) <= 0 && list_count(parent->children) <= 0) {
		TracePrintf(3, "kernel_Wait (ln46): %d called Wait without active or zombie children\n", parent->process_id);
		return(ERROR);
	}


	/* Actual work under */

	// if has dead children, find them and remove them from the 
	// parent->zombie list and global zombies list
	if (list_count(parent->zombies) > 0) {
		TracePrintf(6, "kernel_Wait: found zombie children\n");
		// pop from parent list
		pcb *child_pcb = list_pop(parent->zombies);
		child_pid_retval = child_pcb->process_id;

		// if my understanding is correct, status should be 
		// the pointer to the process
		*status_ptr = =*((int *) child_pcb);
		// remove from global list (found in kernel.h)
		list_remove(zombies, child_pcb);


		// return the pid of the child that returned (mind == blown)
		TracePrintf(3, "kernel_Wait ### end \n");
		return child_pid_retval; 
	}
	

	// if we have live kids waiting, 
	// modify the same block structure we used for delay
	parent->block->is_active = ACTIVE;
	parent->block->type = WAIT_BLOCK;
	// data will point to the blocking process (in this case parent)
	parent->block->data = (void *) parent;
	 
	/* book keeping */
	list_add(blocked_procs, parent);

	// switch to other process if needed
	if (count_items(ready_procs) <= 0) {
		TracePrintf(3, "kernel_Wait: no items on the ready queue \n");
		exit(ERROR);
	}
	else {
		if (goto_next_process(uc, 0) != SUCCESS) {
			TracePrintf(3, "kernel_Wait: failed to kontext switch");
			exit(ERROR);
		}
	}

	// at thois point, the only scenario is that we are returning from being
	// blocked so her we removing exited child and removing it 


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
	if (u_addr > (unsigned int) (stack_bottom << PAGESHIFT) || 
		u_addr < (unsigned int) (heap_top << PAGESHIFT)){
		TracePrintf(1, "Brk: address requestet out of bounds \n");
		return ERROR;
	}

	// heap
	for (i = heap_bottom; i<= heap_top; i++){
		if ((*(curr_proc->region1_pt + i)).valid != 0x1) {
			if (list_count(empty_frame_list) < 1){
				TracePrintf(2, "Brk: out of memory");
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
	//If id is cvar
	// CvarDestory(id);
	// // if id is lock
	// LockDestory(id);
	// // if id is pipe
	// PipeDestroy(id);
	return 0;
}

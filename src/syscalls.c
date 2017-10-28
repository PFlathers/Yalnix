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
int kernel_Fork(UserContext *user_context)
{
	TracePrintf(3, "kernel_Fork ### start \n");

	int i;
	int pfn_scratch;

	pcb *child; // local pcb of child fprocess
	pcb *parent; // local pcb of parent

	// store user context of parent (important for sp and pc)
	parent = curr_proc;
	memcpy((void *) parent->user_context, (void *) user_context, sizeof(UserContext));

	
	// create a new process and coppy the uc to it
	TracePrintf(6, "kernel_Fork: creating new procees\n")
	child = new_process(parent->user_context);
	// allocate space for stack and PTE
	child->region1_pt = (struct pte *) malloc (VREG_1_PAGE_COUNT *sizeof(struct pte));
	ALLOC_CHECK(child->region1_pt, "kernel_Fork");
	child->region0_pt = (struct pte *) malloc(KERNEL_PAGE_COUNT * sizeof(struct pte));
	ALLOC_CHECK(child->region0_pt, "kernel_Fork");
	
	// finish pcb with usefull information
	child->heap_start_pg = parent->heap_start_pg;
	child->brk_address = parent->brk_address;
	TracePrintf(6, "kernel_Fork: done creating new procees\n")

	// if successfull, create pagetables
	TracePrintf(6, "kernel_Fork: memcpy and allocate pagetables\n")
	// copy old ones
	memcpy((void *) child->region1_pt, (void *) parent->region1_pt, (VREG_1_PAGE_COUNT * sizeof(struct pte)) );
	memcpy((void *) child->region0_pt, (void *) parent->region0_pt, (KERNEL_PAGE_COUNT * sizeof(struct pte)) );

	// allocate new physical frames for kernel
	if (list_count(empty_frame_list) =< KERNEL_PAGE_COUNT){
		TracePrintf(1, "kernel_Fork: not enough memory for child's kernel stack");
		exit(ERROR);
	}
	for (i = 0; i < KERNEL_PAGE_COUNT; i++){
		pfn_scratch =  (int) list_pop(empty_frame_list);
		(*(child->region0_pt + i)).pfn = ((u_long) (((int)(pfn_scratch) * PAGESIZE) >> PAGESHIFT));
	}

	// -//- for region 1
	if (list_count(empty_frame_list) <= VREG_1_PAGE_COUNT){
		TracePrintf(1, "kernel_Fork: not enough memory for child's user memory");
		exit(ERROR);
	}
	for (i = 0; i<VREG_1_PAGE_COUNT; i++){
		if ( (*(child->region1_pt + i)).valid == (u_long) 0x1 ){
			list_count(empty_frame_list <= 0){
				TracePrintf(1, "kernel_Fork: not enough memory for child's user memory");
				exit(ERROR);
			}

			pfn_scratch = pop(empty_frame_list);
			(*(child->region1_pt + i)).pfn = ((u_long) (((int)(pfn_scratch) * PAGESIZE) >> PAGESHIFT));
		}
		else{
			(*(child->region1_pt + i)).pfn = (u_long) 0x0;
		}
	}
	TracePrintf(6, "kernel_Fork: done with memcpy and allocate pagetables\n");


	/* copy parent's memory to child process */
	// (don't know how to do it efficiently)

	// this is the one page below the bottom of the stack, I.e.
	// we copy our child's frame here so that we know where
	// to return
	int dest = (KERNEL_STACK_BASE >> PAGESHIFT) - 1;
	unsigned int bcp_dest_pfn = r0_ptlist[dest].pfn;
	unsigned int bcp_dest_val = r0_ptlist[dest].valid;


	// destination should be valid
	r0_ptlist[dest].valid = (u_int) 0x1;


	int src;
	int dst = dest << PAGESHIFT;

	// restore old PTE for destination page
	for (i = 0; i<KERNEL_PAGE_COUNT; i++) {
		// move in the virtual adress of the kernel stack
		src = KERNEL_STACK_BASE + (i*PAGESIZE);

		// destination will have pfn of the child's process page table
		r0_ptlist[dest].pfn = (*(child->region0_pt + i)).pfn;
		WriteRegister(REG_TLB_FLUSH, TLB_FLUSH_0);

		// actually copy the frame
		memcpy((void *) dst, (void *) src, PAGESIZE);
	}

	for (i = 0; i<VREG_1_PAGE_COUNT; i++) {
		if ((*(child->region1_pt + i)).valid == 0x1){
			src = VMEM_1_BASE + (i*PAGESIZE);

			r0_ptlist[dest].pfn = (*(child->region1_pt + i)).pfn;
			WriteRegister(REG_TLB_FLUSH, TLB_FLUSH_0);

			// actually copy the frame
			memcpy((void *) dst, (void *) src, PAGESIZE);
		}
	}

	// restore
	r0_ptlist[dest].pfn = bcp_dest_pfn;
	r0_ptlist[dest].valid = bcp_dest_val;
	WriteRegister(REG_TLB_FLUSH, TLB_FLUSH_0);

	// book keeping
	// add kid to parent's list (potentially init)
	child->parent = parent;
	if (parent->children == NULL){
		parent->children = init_list();
	}
	list_add(parent->children, child);

	// add to all and ready procs
	list_add(all_procs, child);
	list_add(ready_procs, child);

	TracePrintf(6, "kernel_Fork: context switch to new process\n");
	// context switch to the kid
	if (context_switch(parent, child, user_context) != 0){
		TracePrintf(1, "kernel_Fork: error switching failed\n");
		exit(ERROR);
	}
	TracePrintf(6, "kernel_Fork: done with context switch to new process\n");

	TracePrintf(3, "kernel_Fork ### end \n");
	// return
		// if current proc is parent return child id
	if (curr_proc->process_id == parent->process_id){
		return child->process_id;
	}
		// if current proc is child return 0
	else if (curr_proc->procees_id == child->process_id){
		return 0;
	}
		// else return ERROR
	else{
		return ERROR;
	}
}

int kernel_Exec(char *filename, char **argvec)
{
	return 0;
}

void kernel_Exit(int status)
{
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
		*status_ptr = *((int *) child_pcb);
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
		TracePrintf(3, "kernel_Wait: no items on the ready queue - exiting\n");
		exit(ERROR);
	}
	else {
		if (goto_next_process(uc, 0) != SUCCESS) {
			TracePrintf(3, "kernel_Wait: failed to kontext switch - exiting\n");
			exit(ERROR);
		}
	}

	// pc should be pointing here after the wait
	// at thois point, the only scenario is that we are returning from being
	// blocked so her we removing exited child and removing it 
	if (list_count(parent->zombies) < 1) {
		TracePrintf(3, "kernel_Wait: returned after unknown block - exiting\n")
	}

	// remove exited child from the pcb list
	pcb *child_pcb = list_pop(parent->zombies);
	child_pid_retval = child_pcb->process_id;
	
	// remove exited child from the global list
	*status_ptr = *((int *) child_pcb);
	list_remove(zombies, child_pcb)


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

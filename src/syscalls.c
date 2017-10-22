#include "globals.h"
#include "syscalls.h"
#include "pipe.h"
#include "lock.h"
#include "cvar.h"


int Fork()
{
	return 0;
}

int Exec(char *filename, char **argvec)
{
	return 0;
}

void Exit(int status)
{
}

int Wait(int * status_ptr)
{
	return 0;
}

int GetPid()
{
	return 0;
}


/* Brk
 * kernel implementation of the brk library call
 * it check necessary conditions and set the brk address
 * of the new proccess to addr if suitable
 */
int Brk(void *addr)
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
	unsigned int stack_top = ((VREG_1_LIMIT >> PAGESHIFT) - VREG_1_PAGE_COUNT);



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
			if (list_count(&empty_frame_list) < 1){
				TracePrintf(2, "Brk: out of memory");
				return ERROR;
			}

			// all heap pages are valid, rw, and theis phisical number is based on page id
			(*(curr_proc->region1_pt + i)).valid = (u_long) 0x1;
			(*(curr_proc->region1_pt + i)).prot = (u_long) (PROT_READ | PROT_WRITE);
			Node *node = list_pop(&empty_frame_list);
			page_id = (int) node->data;
			(*(curr_proc->region1_pt + i)).pfn = (u_long) ( (PMEM_BASE + 
                  (page_id * PAGESIZE) ) >> PAGESHIFT);

			free(node);
		}
	} // end heap traversal

	// heap to stack
	for ( i = heap_top; i < stack_bottom; i++){
		if ((*(curr_proc->region1_pt + i)).valid == 0x1) {
			(*(curr_proc->region1_pt + i)).valid = (u_long) 0x0;
			unsigned int idx = ((*(curr_proc->region1_pt + i)).pfn << PAGESHIFT) / PAGESIZE;
			list_add(&empty_frame_list, idx);
		}
	}

	curr_proc->brk_addr = heap_top << PAGESHIFT;
	return SUCCESS;
}

int Delay(int clock_ticks)
{
	return 0;
}

/* 
 * Destroys the lock, cvar, or pipe identified by the id, and releases any
 * associated resources. In case of error, the value ERROR is returned.
 */
int Reclaim(int id)
{
	//If id is cvar
	CvarDestory(id);
	// if id is lock
	LockDestory(id);
	// if id is pipe
	PipeDestroy(id);
	return 0;
}

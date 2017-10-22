#ifndef _PCB_H_
#define _PCB_H_

#include <hardware.h>
#include <yalnix.h>
#include "lock.h"
#include "list.h"

// #include wherever User and kernel context are

typedef struct _PCB { 

	/* Process tracking */
	
	unsigned int process_id;
	UserContext *user_context;
	KernelContext *kernel_context;


	//lock *process_lock;

	/* Hierarchy structure */
	// if parent
	List *children;
	List *zombiez;

	// if child
	struct _PCB *parent; //we need to know new 
	int exit_status;

	/* Process Memory management */
	
	int brk_address;

	int stack_pointer; // or void *stack_start
	int heap_start_pg; //or void *heap_start

	// pointers to the kernel and user stack;
	// allocte when creating the process or bruno angry
	// and os segfaulty
	struct pte *region0_pt;
	struct pte *region1_pt;
	
} _PCB;
typedef struct _PCB pcb;



/* Public Facing Function Calls */
// pcb new_process(UserContext\KernelContext);

#endif

#ifndef _PCB_H_
#define _PCB_H_

#include "lock.h"
#include "list.h"
// #include wherever User and kernel context are

typedef struct _PCB { 

	/* Process tracking */
	
	unsigned int process_id;
	//User Context *user_context;
	//Kernel Context *kernel_context;
	lock *process_lock;

	/* Hierarchy structure */
	// if parent
	node *children;
	node *zombiez;

	// if child
	struct _PCB *parent; //we need to know new 
	int exit_status;

	/* Process Memory management */

	int brk_address;
	int neap_start_pg; //or void *heap_start
	
} _PCB;
typedef struct _PCB pcb;



/* Public Facing Function Calls */


#endif

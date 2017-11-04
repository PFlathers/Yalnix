#ifndef _PCB_H_
#define _PCB_H_

#include <hardware.h>
#include <yalnix.h>
#include "lock.h"
#include "list.h"





/*
linux has blocked signal hanler
dtype, 
this is a fake attempt to copy that
https://web.cs.wpi.edu/~claypool/courses/3013-A03/samples/linux-pcb.c
*/

#define INACTIVE    ((u_long) 0x0)
#define ACTIVE      ((u_long) 0x1)

#define NO_BLOCK ((u_long) 0x0)
#define DELAY ((u_long) 0x1)
#define WAIT ((u_long) 0x2)

typedef struct _DELAY {
	// we are either blocking or not
	union{
		int count; // count of the deyay
		int ret; // return value of the blocking party
	} stats;

	u_long is_active; // check if active
	u_long type; // check what kind for sys handler

	void *data;
} _DELAY;

typedef struct _DELAY DelayHandler;

typedef struct _PCB { 

	/* Process tracking */
	
	unsigned int process_id;
	UserContext *user_context;
	KernelContext *kernel_context;
	int has_kc;

	// see above
	DelayHandler *block;
	// unsigned int clock_ticks;
	// unsigned int timeflag = 0;
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


	int pipe_lenght;
	
} _PCB;
typedef struct _PCB pcb;



/* Public Facing Function Calls */
pcb *new_process(UserContext *uc);
int check_block_status(DelayHandler *block);

#endif

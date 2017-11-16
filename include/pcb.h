/*=============================================================================
 |   Assignment:   Yalnix Operating System
 |
 |       Author:  P.J.Flathers and B.Korbar
 |     Language:  C
 |   To Compile:  part of the yalnix os --- see makefile in the root folder
 |
 |        Class:  COSC 58 - Operating Systems
 |   Instructor:  Dr. Sean Smith
 |     Due Date:  Nov 16th, 2017
 |
 +-----------------------------------------------------------------------------
 |
 |  Purpose:  header for the PCB implementation; source can be found in 
 | 		../src/pcb.c
 |
 *===========================================================================*/


#ifndef _PCB_H_
#define _PCB_H_

/* 
 * Sys Includes
 */
#include <hardware.h>
#include <yalnix.h>
/* 
 * Local includes
 */
#include "lock.h"
#include "tty.h"
#include "list.h"


/* 
 * DATA STRUCTURES
 */


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



/* 
our implementation of the PCB is mostly guided
by the manual, with a bunch of Sean's input on hte 
first couple office hours, iterated many times 
over the couse of the project
*/
typedef struct _PCB { 

	/* Process tracking */
	unsigned int process_id;			     // pid
	UserContext *user_context;          // user context related 
	KernelContext *kernel_context;    // kernel context related 
	int has_kc;  // boolean flag telling us if we have copied kc

	// see DS above - rather than having
	// clock_ticks and lock DS, we can generalize
	// all into one
	DelayHandler *block;


	/* Hierarchy structure */

	List *children;	// list of active children
	List *zombiez;	// list of exited pchildren

	// if child
	struct _PCB *parent; // pcb of the parent
	int exit_status; // exit status for the 
			 // parent to know its faith

	/* Process Memory management */
	unsigned int brk_address; // top of the heap +1

	unsigned int stack_pointer; // or start of the stack ptr
	int heap_start_pg; //or last heap page

	// pointers to the kernel and user stack;
	// allocte when creating the process or bruno angry
	// and os segfaulty
	struct pte *region0_pt;
	struct pte *region1_pt;

	// buffer and tty realted messyness
	Buffer *buffer;
	int read_length; // do we have things to read
	int pipe_lenght; // how much do we have in a pipe
	
} _PCB;
typedef struct _PCB pcb;


/* Public Facing Function Calls */


/*------------------------------------------------- new_process -----
|  Function new_process
|
|  Purpose:  Allocates and creates the PCB for the new process
|
|  Description: all lists and buffers init to NULL, all scalars
|  		Init to 0; when creating, ALLOCATE PTE POINTERS
|
|  Parameters:
|      User Context uc (IN/OUT) - current user context, 
|   				it is returned as a part of PCB
|
|  Returns:  pointer to the PCB of the newly created process
*-------------------------------------------------------------------*/
pcb *new_process(UserContext *uc);


/*------------------------------------------------- check_block_status -----
|  Function check_block_status
|
|  Purpose:  Check if the process is blocking; used only for delay 
|
|  Parameters:
|      DelayHandler *block (IN) -- pointer to the DelayHandler datastructure
| 				usually found in the pcb as pcb->block
|
|  Returns:  returns 0 if block is inactive, 1 if active and ERROR 
| 	     otherwise
*-------------------------------------------------------------------*/
int check_block_status(DelayHandler *block);

#endif

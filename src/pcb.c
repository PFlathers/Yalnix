/*
 * PCB.c
 * the only function we should need here is 
 * the "new_process function", which would take
 * either user or kernel context (if we can take kernel context)
 * and return a pcb for it
 */

#include <hardware.h>


/*
 * Local includes
 */
#include "globals.h"
#include "tty.h"
#include "pcb.h"
#include "kernel.h"


/* Prototype new_process function */

pcb *new_process(UserContext *uc)
{

	// malloc new pcb
	pcb *new_pcb = (pcb *) malloc( sizeof(pcb) );
	ALLOC_CHECK(new_pcb, "PCB: new_process");


	// TODO: alloc check
	// malloc user context for the pcb
	new_pcb->user_context = (UserContext *) malloc( sizeof(UserContext) );
	// malloc kernel context for the pcb
	new_pcb->kernel_context = (KernelContext *) malloc(sizeof(KernelContext));

	// malloc lock for the pcb
	new_pcb->block = (DelayHandler *) malloc( sizeof(DelayHandler));
	bzero((char *)new_pcb->block, sizeof(DelayHandler));

	
	// here I follow page 36 of the manual:

	// newpcb inherits vector pointer, and code 
	// vector is the index into the interrupt vector table used to 
	// call this handler function.
	new_pcb->user_context->vector = uc->vector;
	//The meaning of the code value varies depending 
	// on the type of interrupt, exception, or trap. 
	// The defined values of code are summarized in Table 3.2.
	new_pcb->user_context->code = uc->code;

	// set new proc id - this is a global from kernel.h
	new_pcb->process_id = available_process_id;
	available_process_id++;
	// modify user/kernel context? 

	// initialize all pcb fields 
  	new_pcb->children = NULL;
  	new_pcb->zombiez = NULL;
  	new_pcb->parent = NULL;

  	new_pcb->brk_address = 0;
  	new_pcb->heap_start_pg = 0;


	return new_pcb;


}



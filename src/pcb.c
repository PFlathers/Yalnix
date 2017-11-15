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
 |  Purpose:  implements the operations on the PCB
 |
 *===========================================================================*/



/*
 * System includes
 */
#include <hardware.h>

/*
 * Local includes
 */
#include "globals.h"
#include "tty.h"
#include "pcb.h"
#include "kernel.h"


/*------------------------------------------------- new_process -----
|  Function new_process
|
|  Purpose:  Allocates and creates the PCB for the new process
|
|  Parameters:
|      User Context uc (IN/OUT) - current user context, 
|   				it is returned as a part of PCB
|
|  Returns:  pointer to the PCB of the newly created process
*-------------------------------------------------------------------*/
pcb *new_process(UserContext *uc)
{
	TracePrintf(3, "new_process ### enter\n");
	// malloc new pcb
	pcb *new_pcb = (pcb *) malloc( sizeof(pcb) );
	ALLOC_CHECK(new_pcb, "PCB: new_process");


	// TODO: alloc check
	// malloc user context for the pcb
	new_pcb->user_context = (UserContext *) malloc( sizeof(UserContext) );
	// malloc kernel context for the pcb
	new_pcb->kernel_context = (KernelContext *) malloc(sizeof(KernelContext));
	new_pcb->has_kc = 0; // kernel context has to be copied over

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
	TracePrintf(6, "new_process: current proc id is %d, next one will be %d\n", new_pcb->process_id, available_process_id);

	// modify user/kernel context? 

	// initialize all pcb fields 
  	new_pcb->children = NULL;
  	new_pcb->zombiez = NULL;
  	new_pcb->parent = NULL;

  	new_pcb->brk_address = 0;
  	new_pcb->heap_start_pg = 0;
  	new_pcb->pipe_lenght = 0;


  	// messing with buffers and pipe things
  	new_pcb->buffer = (Buffer *) malloc(sizeof(Buffer));
  	ALLOC_CHECK(new_pcb->buffer, "new_process - alloc buffer");
  	
  	new_pcb->buffer->len = 0;
  	new_pcb->buffer->buf = NULL;


	TracePrintf(3, "new_process ### end\n");

	return new_pcb;
}

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
int check_block_status(DelayHandler *block)
{
	unsigned int ticks;
	pcb *proc_on_delay;

	// if block is inactive
	if (block->is_active == INACTIVE){
		return(0);
	} // if it's neither active or inactive
	else if (block->is_active != ACTIVE){
		return(ERROR);
	}

	// if there is no block, zero out the memory, and
	// return NO_BLOCK
	if (block->type == NO_BLOCK) {
		bzero((char *) block, sizeof(DelayHandler));
		return(0);
	}

	// if the block is of the type delay, it has a ticks 
	// count -> find that count and decrement it as we would
	// call this from the CLOCK trap 
	if (block->type == DELAY){
		ticks = (unsigned int) block->stats.count;

		if (ticks > 1) {
			block->stats.count--;
			return 1;
		}
		else{
			// if we are done blocking, zero out the memory, and
			// return NO_BLOCK
			bzero((char *) block, sizeof(DelayHandler));
			return(0);
		}
	}
	else{

		return(ERROR);
	}

	return(ERROR);

}


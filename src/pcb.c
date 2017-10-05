/*
 * PCB.c
 * the only function we should need here is 
 * the "new_process function", which would take
 * either user or kernel context (if we can take kernel context)
 * and return a pcb for it
 */

/*
 * Local includes
 */
#include "tty.h"
#include "pcb.h"

/* Prototype new_process function

pcb new_process(UserContext or kenel context?)
{

	// malloc new pcb
	pcb *new_pcb = (pcb *) malloc( sizeof(pcb) );

	// malloc user context for the pcb
	// malloc kernel context for the pcb

	// malloc lock for the pcb
	pcb->process_lock = (lock *) malloc( sizeof(lock) );


	// set new proc id

	// modify user/kernel context? 

	// initialize all pcb fields 
  	new_pcb->children = NULL;
  	new_pcb->zombiez = NULL;
  	new_pcb->parent = NULL;

	return new_pcb;


}


*/
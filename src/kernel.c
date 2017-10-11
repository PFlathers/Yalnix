#include "kernel.h"
#include "globals.h"
#include "list.h"

// functions from assignment 


/* 
 * KernelStart
 *
 * PseudoCode:
 *  - Initalize global variables;
 */
void KernelStart(char *cmd_args[], 
                 unsigned int phys_mem_size,
                 UserContext *user_context) 
{
	TracePrintf(1, "Start: KernelStart \n");

	/*LOCAL VARIABLES*/
	int i; 
	int arg_count; // number of variables passed 


	/*GLOBAL VARIABLES*/

	available_process_id = 0;	// at start we run at 0

	kernel_brk = kernel_data_end;	// break starts as kernel_data_end
	available_process_id = 0;	// PIDs start at 0

	ready_procs = (list *)init_list();
	blocked_procs = (list *)init_list();
	all_procs = (list *)init_list(); 
	zombie_procs = (list *)init_list(); 




} 

/*SwitchContext*/
/*SetBreak*/
/*Change Process*/
/*Switch PCBs*/
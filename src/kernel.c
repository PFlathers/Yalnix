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



// Create the list of empty frames

// Set up the page tables for what's already in use by the kernel.

	// Create an empty page table entry structure and assign permissions and validity

	// save pagetable 0

	// Create pte for r1 pagetables

	// Create an empty page table entry structure and assign permissions and validity

	// Save pagetable 1 (

// Create the idle process
	// Create a shell process
	// Set up the user stack by allocating two frames
	// Update the 1st page table with validity & pfn of idle
	// assign values to the PCB structure and UserContext for DoIdle

	// Assign Idle PCB


} 

/*SwitchContext*/
/*SetBreak*/
/*Change Process*/
/*Switch PCBs*/
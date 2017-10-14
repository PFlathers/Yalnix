#include "kernel.h"
#include "globals.h"
#include "list.h"

// Interrupt vector (pg50, bull 1)
void (*interrupt_vector[8]) = {
  HANDLE_TRAP_KERNEL, 
  HANDLE_TRAP_CLOCK,
  HANDLE_TRAP_ILLEGAL,
  HANDLE_TRAP_MEMORY,
  HANDLE_TRAP_MATH,
  HANDLE_TRAP_TTY_RECEIVE,
  HANDLE_TRAP_TTY_TRANSMIT,
  HANDLE_TRAP_DISK
};

/*KernelStart*/
/* 
 * KernelStart
 *
 * PseudoCode:
 *  Before allowing the execution of user processes, 
 * the KernelStart routine 
 * should perform any initialization necessary 
 * for your kernel or required by the hardware. 
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

	// phisical memory
	// constants in hardware.h
	// memory shifts explained in pg 24, bullet 4
	total_phisical_frames = pmem_size / PAGESIZE;
	physical_kernel_frames = (VMEM_0_LIMIT >> PAGESHIFT);
	used_physical_kernel_frames = UP_TO_PAGE(kernel_brk) >> PAGESHIFT;

	// under is a mess, clean it up

	available_process_id = 0;	// at start we run at 0

	kernel_brk = kernel_data_end;	// break starts as kernel_data_end
	available_process_id = 0;	// PIDs start at 0

	// process tracking lists
	ready_procs = (list *)init_list();
	blocked_procs = (list *)init_list();
	all_procs = (list *)init_list(); 
	zombie_procs = (list *)init_list(); 

	/* PAGE TABLES */
    // Build the initial page tables for Region 0 and Region 1 (pg 50, bullet 4)
	for (i = 0; i < physical_kernel_frames; i++) {
    	// PTE struct
    	struct pte entry;

    	// If counter is above stack base or
  		// below the used frames make it valid
		if ((i < used_physical_kernel_frames) || (i >= (KERNEL_STACK_BASE >> PAGESHIFT)))
		   entry.valid = (u_long) 0x1;
		else
		   entry.valid = (u_long) 0x0; 


		



    }

	/* CONFUGURE REGS */
	// interrupt vector at REG_VEC_BASE (pg50, bullet 2)
  	WriteRegister(REG_VECTOR_BASE, (unsigned int) &interrupt_vector);


, 
    // and initialize the registers REG PTBR0, REG PTLR0, REG PTBR1, 
    // and REG PTLR1 to define these initial page tables.


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

	// Assign Idle PCBs


} 

/*SwitchContext*/
/*SetBreak*/
/*Change Process*/
/*Switch PCBs*/

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


/* from pg 14, bullet 1
At boot time, the hardware invokes SetKernelData 
to tell your kernel some basic pa- rameters about the data segment
*/
void SetKernelData(void * _KernelDataStart, void *_KernelDataEnd) { 
  TracePrintf(1, "Start: SetKernelData \n");

  kernel_data_start = _KernelDataStart; 
  kernel_data_end = _KernelDataEnd;

  TracePrintf(1, "End: SetKernelData \n");
}


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
    // Page table entry is 32-bits wide (but does not use all 32 bits).
	for (i = 0; i < physical_kernel_frames; i++) {
    	// PTE struct
    	struct pte new_pt_entry;

    	// If counter is above stack base or
  		// below the used frames make it valid
		if ((i < used_physical_kernel_frames) || (i >= (KERNEL_STACK_BASE >> PAGESHIFT)))
		   new_pt_entry.valid = (u_long) 0x1;
		else
		   new_pt_entry.valid = (u_long) 0x0; 

		// if memory is bellow kernel data start 
		// (i.e if it's kernel stach), we can read or exec,
		// if userland we can read or write (see pg 28, bullet 2) 
		if (i < (((unsigned int)kernel_data_start) >> PAGESHIFT))
      		new_pt_entry.prot = (u_long) (PROT_READ | PROT_EXEC); // exec and read protections
		else
      		new_pt_entry.prot = (u_long) (PROT_READ | PROT_WRITE); // read and write protections






    }

	/* CONFUGURE REGS */
	// interrupt vector at REG_VEC_BASE (pg50, bullet 2)
  	WriteRegister(REG_VECTOR_BASE, (unsigned int) &interrupt_vector);


, 
    // and initialize the registers REG PTBR0, REG PTLR0, REG PTBR1, 
    // and REG PTLR1 to define these initial page tables.


// Create the list of empty frames

// Set up the page tables for what's already in use by the kernel.

	// Create an empty page table new_pt_entry structure and assign permissions and validity

	// save pagetable 0

	// Create pte for r1 pagetables



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

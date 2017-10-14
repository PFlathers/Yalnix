#include "kernel.h"
#include "globals.h"
#include "list.h"
#include <hardware.h>
#include "interupts.h"

// Interrupt vector (pg50, bull 1)
void (*interrupt_vector[TRAP_VECTOR_SIZE]) = {
  HANDLE_TRAP_KERNEL		= &trapKernel,
  HANDLE_TRAP_CLOCK		= &trapClock,
  HANDLE_TRAP_ILLEGAL		= &trapIllegal,
  HANDLE_TRAP_MEMORY		= &trapMemory,
  HANDLE_TRAP_MATH		= &trapMath,
  HANDLE_TRAP_TTY_RECEIVE	= &trapTTYReceive,
  HANDLE_TRAP_TTY_TRANSMIT	= &trapTTYTransmit,
  HANDLE_TRAP_DISK		= &trapname1,
  HANDLE_TRAP_2			= &trapname2,
  HANDLE_TRAP_3			= &trapname3,
  HANDLE_TRAP_4			= &trapname4,
  HANDLE_TRAP_5			= &trapname5,
  HANDLE_TRAP_6			= &trapname6,
  HANDLE_TRAP_7			= &trapname7,
  HANDLE_TRAP_8			= &trapname8,
  HANDLE_TRAP_9			= &trapname9
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

	// lowest frame in region 1 - see figure 2.2
	int r1_base_frame = DOWN_TO_PAGE(VMEM_1_BASE) >> PAGESHIFT;
	// highest frame in region 1 - see 3ipure 2.2
	int r1_top_frame = UP_TO_PAGE(VMEM_1_LIMIT) >> PAGESHIFT;


	/*GLOBAL VARIABLES*/

	// break starts as kernel_data_end
	kernel_brk = kernel_data_end;	

	// phisical memory
	// constants in hardware.h
	// memory shifts explained in pg 24, bullet 4
	total_phisical_frames = pmem_size / PAGESIZE;
	physical_kernel_frames = (VMEM_0_LIMIT >> PAGESHIFT);
	used_physical_kernel_frames = UP_TO_PAGE(kernel_brk) >> PAGESHIFT;


	// Create the list of empty frames (NEEDS EDITING)
  	for (i = physical_kernel_frames; i < total_phisical_frames; i++){
		add_to_list(&empty_frame_list, i);
  	}


	// under is a mess, clean it up

	available_process_id = 0;	// at start we run at 0

	// process tracking lists (per sean's suggestion)
	ready_procs = (list *)init_list();
	blocked_procs = (list *)init_list();
	all_procs = (list *)init_list(); 
	zombie_procs = (list *)init_list(); 



	/* PAGE TABLES */

    // Build the initial page tables for Region 0 (pg 50, bullet 4)
   	// Page table entry is 32-bits wide (but does not use all 32 bits).
	// struct pte defined in hardware.h
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

		// pft (24 bits as defined on pg 28, bullet 3)
		//field contains the page frame number (the physical memory page number) 
		// of the page of physical memory to which this virtual memory page is 
		//mapped by this page table entry
		// If i get this correctly, that should just be base + ith page's address
		new_pt_entry.pfn = (u_long) ((PMEM_BASE + (i * PAGESIZE)) >> PAGESHIFT);
		
		// set up a pagetable entry to be sth like
		r0_ptlist[i] = new_pt_entry;
  
    }

	// Build the initial page tables for Region 1 (pg 50, bullet 4)
	for (i = base_frame_r1; i < top_frame_r1; i++) {
		struct pte new_pt_entry; // New pte entry

		// So far, page is invalid, has read/write protections, and no pfn
		new_pt_entry.valid = (u_long) 0x0;
		new_pt_entry.prot = (u_long) (PROT_READ | PROT_WRITE);
		new_pt_entry.pfn = (u_long) 0x0;

		// Add the page to the pagetable (accounting for 0-indexing - not sure if I need to add 1)
		r1_ptlist[i - base_frame_r1] = new_pt_entry;
	}


	/* CONFUGURE REGS */
	// interrupt vector at REG_VEC_BASE (pg50, bullet 2)
  	WriteRegister(REG_VECTOR_BASE, (unsigned int) &interrupt_vector);

  	// this is my guess based on: pg50, bullet 4
  	// 							  Table 3.3
  	//							  pg 27, bullet 1
	WriteRegister(REG_PTBR0, (unsigned int) &r0_ptlist);
	WriteRegister(REG_PTBR1, (unsigned int) &r1_ptlist);
	WriteRegister(REG_PTLR0, (unsigned int) VMEM_0_PAGE_COUNT);
	WriteRegister(REG_PTLR1, (unsigned int) VMEM_1_PAGE_COUNT); 

	// Enable virtual memory as in table 3.3
	WriteRegister(REG_VM_ENABLE, 1);
	TracePrintf(1, "Virtual Memory Enabled!\n");
	// Flush the tlb as in pg29, bullet 1
	// (when do we not want to have flush all and use other consts?)
	WriteRegister(REG_TLB_FLUSH, TLB_FLUSH_ALL);
	TracePrintf(8, "TLB flushed!\n");



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

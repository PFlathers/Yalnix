#include <hardware.h>

#include "kernel.h"
#include "pcb.h"
#include "globals.h"
#include "list.h"
#include "interupts.h"
#include "loadprog.h"


// Interrupt vector (pg50, bull 1)
void (*interrupt_vector[TRAP_VECTOR_SIZE]) = {
   &trapKernel,
   &trapClock,
   &trapIllegal,
   &trapMemory,
   &trapMath,
   &trapTTYReceive,
   &trapTTYTransmit,
   &trapname1,
   &trapname2,
   &trapname3,
   &trapname4,
   &trapname5,
   &trapname6,
   &trapname7,
   &trapname8,
   &trapname9
}; 

/* from pg 14, bullet 1
At boot time, the hardware invokes SetKernelData 
to tell your kernel some basic pa- rameters about the data segment
*/
void SetKernelData(void * _KernelDataStart, void *_KernelDataEnd) 
{ 
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
	TracePrintf(0, "Start: KernelStart \n");

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

	// physical memory
	// constants in hardware.h
	// memory shifts explained in pg 24, bullet 4
	total_physical_frames = phys_mem_size / PAGESIZE;
	physical_kernel_frames = (VMEM_0_LIMIT >> PAGESHIFT);
	used_physical_kernel_frames = UP_TO_PAGE(kernel_brk) >> PAGESHIFT;

	// init me baybe
	empty_frame_list = init_list();
	// Create the List of empty frames (NEEDS EDITING)
  	for (i = physical_kernel_frames; i < total_physical_frames; i++){
		list_add(empty_frame_list, (void *) i);
  	}


	// under is a mess, clean it up

	available_process_id = 0;	// at start we run at 0


	// process tracking lists (per sean's suggestion)
	ready_procs = (List *)init_list();
	blocked_procs = (List *)init_list();
	all_procs = (List *)init_list(); 
	zombie_procs = (List *)init_list(); 



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
	for (i = r1_base_frame; i < r1_top_frame; i++) {
		struct pte new_pt_entry; // New pte entry

		// So far, page is invalid, has read/write protections, and no pfn
		new_pt_entry.valid = (u_long) 0x0;
		new_pt_entry.prot = (u_long) (PROT_READ | PROT_WRITE);
		new_pt_entry.pfn = (u_long) 0x0;

		// Add the page to the pagetable (accounting for 0-indexing - not sure if I need to add 1)
		r1_ptlist[i - r1_base_frame] = new_pt_entry;
	}


	/* CONFUGURE REGS */

	// interrupt vector at REG_VEC_BASE (pg50, bullet 2)
  	WriteRegister(REG_VECTOR_BASE, (unsigned int) &interrupt_vector);

  	// this is my guess based on: pg50, bullet 4
  	// 							  Table 3.3
  	//							  pg 27, bullet 1
	WriteRegister(REG_PTBR0, (unsigned int) &r0_ptlist);
	WriteRegister(REG_PTBR1, (unsigned int) &r1_ptlist);
	WriteRegister(REG_PTLR0, (unsigned int) VREG_0_PAGE_COUNT);
	WriteRegister(REG_PTLR1, (unsigned int) VREG_1_PAGE_COUNT); 

	// Enable virtual memory as in table 3.3
	WriteRegister(REG_VM_ENABLE, 1);
	TracePrintf(1, "Virtual Memory Enabled!\n");
	
	// Flush the tlb as in pg29, bullet 1
	// (when do we not want to have flush all and use other consts?)
	WriteRegister(REG_TLB_FLUSH, TLB_FLUSH_ALL);
	TracePrintf(8, "TLB flushed!\n");


	/* 
	 * PROCESS HANDLING 
	 * IDLE
	 */

	// Create idle process based on current user context
	// allocates within the function (see pcb.c)
	// global idle_proc pcb defined in kernel.h
           
	idle_proc = (pcb *) new_process(user_context); 

	// take two frames
	Node *node = list_pop(empty_frame_list);
	if (!node){
		exit(-42);
	}
	int idle_stack_frame1 = (int)node->data;     
	free(node);
	node = list_pop(empty_frame_list);  
	if (!node){
		exit(-42);
	}        
	int idle_stack_frame2 = (int) node->data; 
	free(node);

 	// we know that the idle will take the first two pages
 	// in user region (1); and we can set the bits to valid
  	r1_ptlist[VREG_1_PAGE_COUNT - 1].valid = (u_long) 0x1;
 	r1_ptlist[VREG_1_PAGE_COUNT - 2].valid = (u_long) 0x1; 

 	r1_ptlist[VREG_1_PAGE_COUNT - 1].pfn = (u_long) ((idle_stack_frame1 * PAGESIZE) >> PAGESHIFT);
 	r1_ptlist[VREG_1_PAGE_COUNT - 2].pfn = (u_long) ((idle_stack_frame2 * PAGESIZE) >> PAGESHIFT);

	// flush the TLB region 1 after messing with it
	WriteRegister(REG_TLB_FLUSH, TLB_FLUSH_1);


	// ASSIGN PCB VALUES TO DO IDLE
	//program counter to do idle:
	idle_proc->user_context->pc = &DoIdle;
	// since this is the first page, stack pointer addr can be 
	// just the upper limit minus one page and it grows downwardes
	idle_proc->user_context->sp = (void *) (VMEM_1_LIMIT - PAGESIZE);
	// allocated region1 pageTable is just a pagecount for region 1 process 
	// times the size of the pte struct
	idle_proc->region1_pt = (struct pte *)malloc( VREG_1_PAGE_COUNT * sizeof(struct pte));
	// Copy over idle's region 1 page table
	memcpy((void *)idle_proc->region1_pt, (void *) r1_ptlist, VREG_1_PAGE_COUNT * sizeof(struct pte));


	// region0 pagetable is malloced in the same way as the user one
	idle_proc->region0_pt = (struct pte *)malloc( KERNEL_PAGE_COUNT * sizeof(struct pte));
	// this 
	memcpy((void *)idle_proc->region0_pt,
		  (void *) &(r0_ptlist[KERNEL_STACK_BASE >> PAGESHIFT]), 
		  KERNEL_PAGE_COUNT * sizeof(struct pte));


	/* 
	 * PROCESS HANDLING 
	 * INIT
	 */

	 // base it of of the init (so that I don't have to re-do things)
	 pcb *init_proc = (pcb *) new_process(user_context);

	 init_proc->region0_pt = (struct pte *)malloc(KERNEL_PAGE_COUNT * sizeof(struct pte));
	 init_proc->region1_pt = (struct pte *)malloc(VREG_1_PAGE_COUNT * sizeof(struct pte));
	 // zero out the memory for pt1
	 //bzero((char *)(init_proc->region1_pt), VREG_1_PAGE_COUNT * sizeof(struct pte));

	 // memory in region 1 should be invalid, 
	 for (i=0; i < VREG_1_PAGE_COUNT; i++) {
	 	(*(init_proc->region1_pt + i)).valid = (u_long) 0x0;
	 	(*(init_proc->region1_pt + i)).prot = (u_long) (PROT_READ | PROT_WRITE);
	 	(*(init_proc->region1_pt + i)).pfn = (u_long) 0x0;
	 }



	 for (i = 0; i < KERNEL_PAGE_COUNT; i++){
	 	(*(init_proc->region0_pt + i)).valid = (u_long) 0x1;
    	(*(init_proc->region0_pt + i)).prot = (u_long) (PROT_READ | PROT_WRITE);
    	node = (Node *) list_pop(empty_frame_list);
    	// this is sketchy
    	(*(init_proc->region0_pt + i)).pfn = (u_long) (((int)node->data * PAGESIZE) >> PAGESHIFT);;
	 	free(node);
	 }

	// TLB flush
	WriteRegister(REG_TLB_FLUSH, TLB_FLUSH_ALL);

 	// init_proc is the first child of idle_proc!
	init_proc->parent = idle_proc;
  	idle_proc->children = init_list();
  	list_add(idle_proc->children, (void *)init_proc);

  	if (cmd_args[0] == NULL) {
  		list_add(all_procs, (void *)idle_proc);

  		curr_proc = idle_proc;


  		// copy idle's UC into the current UC
  		memcpy(user_context, idle_proc->user_context, sizeof(UserContext));
  		TracePrintf(2, "KernelStart ### End");

  	} else{
  		int arg_count = 0;

  		while (cmd_args[arg_count] != NULL){
  			arg_count++;
  		}
  		// null terminated
  		arg_count++; 

  		char *argument_list[arg_count];
  		for (i = 0; i<arg_count; i++){
  			argument_list[i] = cmd_args[i];
  		}

  		char *program_name = argument_list[0];

  		int lpr;
  		if ( (lpr = LoadProgram(program_name, argument_list, init_proc)) == SUCCESS ) {
  			list_add(all_procs, (void *) init_proc);
  			list_add(ready_procs, (void*) init_proc);
  		} else{
  			WriteRegister(REG_PTBR1, (unsigned int) &r1_ptlist);
  			WriteRegister(REG_TLB_FLUSH, TLB_FLUSH_ALL);
  			TracePrintf(3, "LoadProgram failed; code %d", lpr);
  		}
  	}

	/* 
	 * BOOKKEEPING 
	 */

	list_add(all_procs, (void *) idle_proc);
	curr_proc = idle_proc;
	// copy idle's usercontext into the current usercontext
	memcpy(user_context, idle_proc->user_context, sizeof(UserContext));


	TracePrintf(1, "End: SernelStart\n");

} 

/*
	SetKernelBrk
	increase kenrnel break to addr if addr is larger than 
	our current break in VM

	if VM is not enabled, we neeg to realign things and 
	it becomes a pain in the arse; essentially, we need
	to rewrite page table permissions to ensure that everything up
	to the new brk becomes valid (as it might not have been)
 */
int SetKernelBrk(void * addr) 
{
	int i;
	TracePrintf(2, "SetKernelBrk ### Start");
	
	// Check that the address is within bounds
	

	// get VM status from register
	unsigned int vm_enabled = ReadRegister(REG_VM_ENABLE);


	// check if VM is enabled
	if (vm_enabled){
		// if yes pageshift VMEM_0_BASE
		unsigned int page_bottom = VMEM_0_BASE >> PAGESHIFT;
		// page adress is pageshifted toPage(addr)
		unsigned int page_addr = DOWN_TO_PAGE(addr) >> PAGESHIFT;
		// botom of the stack is pageshifted toPage(KERNEL BASE)
		unsigned int stack_bottom = DOWN_TO_PAGE(KERNEL_STACK_BASE) >> PAGESHIFT;

		// from bottom to addr of page, update validity as (u_long) 0x01
		for (i = page_bottom; i <= page_addr; i++){
			r0_ptlist[i].valid = (u_long) 0x1;
		}

		// from addr of pade to bottom of stack invalid
		for (i  = page_addr; i< stack_bottom; i++){
			r0_ptlist[i].valid = (u_long) 0x0;
		}
		
		// brk = addr
		kernel_brk = addr;
		
		// flush tlb (only region 0, see hardware.h)
		WriteRegister(REG_TLB_FLUSH, TLB_FLUSH_0);

	} 
	else{ // else keep track of the higthes requested address
		unsigned int u_addr = (unsigned int) addr;
		unsigned int u_brk = (unsigned int) kernel_brk;
		// add safety check for casts

		// if address is larger than the previous break, set new break, 
		// else we do nothing
		if ( u_addr > u_brk ){
			kernel_brk = addr;
		}


	} // end if(vm_enabled)
	
	// exit function
	TracePrintf(2, "SetKernelBrk ### End");
	return 0;

}

/* given idle function */
void DoIdle() {
  while (1) {
    TracePrintf(1, "\tDoIdle\n");
    Pause();
  } 
} 


/* magic function from 5.2 */ 
KernelContext *MyKCS(KernelContext *kernel_context_in, void *current_pcb, void *next_pcb)
{
	//casts
	pcb *current = (pcb *) current_pcb;
	pcb *next = (pcb *) next_pcb; 

	// save current kc
	if (current != NULL){
		memcpy( (void *) (current->kernel_context), (void *) kernel_context_in, sizeof(KernelContext));
	}

	// close current on how should we ? 

	// save current k stack
	if (current != NULL) {
      memcpy((void *) current->region0_pt,
              (void *) (&(r0_ptlist[KERNEL_STACK_BASE >> PAGESHIFT])),
              KERNEL_PAGE_COUNT * (sizeof(struct pte)));
    }


    // get the region's kernel stack
    memcpy((void *) (&(r0_ptlist[KERNEL_STACK_BASE >> PAGESHIFT])),
            (void *) next->region0_pt,
            KERNEL_PAGE_COUNT * (sizeof(struct pte)));

    // update pt register
    WriteRegister(REG_PTBR1, (unsigned int) next->region1_pt);

    // flush the TLB
    WriteRegister(REG_TLB_FLUSH, TLB_FLUSH_ALL);

    // return
    return next->kernel_context;

}

/* switch to a new process - makes life easier for syscalls */
goto_next_process(UserContext *user_context, int repeat_bool)
{

	if (repeat_bool) {
		list_add(ready_procs, (void *) curr_proc);
	}

	// context switching
	Node *node;
	node = list_pop(ready_procs);
	pcb *next_proc = node->data;
	free(node);

	if (context_switch(curr_proc, next_proc, user_context) != 0){
		exit(FAILURE);
	}


	exit(SUCCESS);
}


int context_switch(pcb *current, pcb *next, UserContext *user_context)
{
	// save UC of the current one
	if (current != NULL){
		memcpy((void *)current->user_context, (void *) user_context, sizeof(UserContext));
	}

	// Save hext process' UC in the currently used uc var so that we don't have to 
	// reallocate (tip from prof Palmer in CS50 :))
	memcpy((void *) user_context, (void *) next->user_context, sizeof(UserContext));

	// current procces becomes next
	curr_proc = next;

	// magic function from 5.2
	int r = KernelContextSwitch(MyKCS, (void *) current, (void *) next);

	// make user context current one (not needed atm)
	memcpy((void *) user_context, (void *) curr_proc->user_context, sizeof(UserContext));

	// return status from the magic function
	return r;
}	


/*SwitchContext*/
/*SetBreak*/
/*Change Process*/
/*Switch PCBs*/

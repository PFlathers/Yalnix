/*=============================================================================
 |   Assignment:  Yalnix Kernel
 |
 |       Author:  Patrick Flathers and Bruno Korbar
 |     Language:  C compiled by Sean's magical 
 |   To Compile:  run `make` in the source folder; find results in bin folder
 |
 |        Class:  COSC58
 |   Instructor:  Sean Smith
 |     Due Date:  too soon
 |
 +-----------------------------------------------------------------------------
 |
 | Description:  Through this assignment, you will be able to learn how a real 
 | operating system kernel works, managing the hardware resources of the computer 
 | system and providing services to user processes running on the system. In the 
 | project, you will implement an operating system kernel for the Yalnix operating 
 | system, running on a fictional computer system known as the DCS 58.
 |
 |   Known Bugs:  it doesn't really work (yet)
 |
 *===========================================================================*/

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
  TracePrintf(1, "SetKernelData ### Start\n");

  kernel_data_start = _KernelDataStart; 
  kernel_data_end = _KernelDataEnd;

  TracePrintf(1, "SetKernelData ### End\n");
}


/*
 * Initializes the process lists which are: ready_procs, all_procs, blocked_procs, zombie_procs
 * Initializes the empty_frame list so we can create pagetabels later.
 * Sets the first process id to 0.
 *
 * function variables: 
 *      phy_mem_size: the actual size of how much ram we have to work with.
 */
void init_global(int phys_mem_size)
{
	int frame_iter;
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
  	for (frame_iter = physical_kernel_frames; frame_iter < total_physical_frames; frame_iter++){
		list_add(empty_frame_list, (void *) frame_iter);
  	}


	available_process_id = 0;	// at start we run at 0
        glob_resource_list = 0;         // for pipes and stuff
        available_lock_id = 0;


	// process tracking lists (per sean's suggestion)
	ready_procs = (List *)init_list();
	blocked_procs = (List *)init_list();
	all_procs = (List *)init_list(); 
	zombie_procs = (List *)init_list(); 
        locks = (List *)init_list();
        cvars = (List *) init_list();


        // book kepingz
        pipes = (List *)init_list();

        // init ttys - checkpoint 5
        ttys = init_list();
        int tty_iter;
        for (tty_iter = 0; tty_iter < NUM_TERMINALS; tty_iter++){
                TTY *tempTTY = (TTY *) malloc(sizeof(TTY));
                tempTTY->buffers = init_list();
                tempTTY->to_read = init_list();
                tempTTY->to_write = init_list();
                tempTTY->tty_id = tty_iter;
                list_add(ttys, (void *) tempTTY);
        }
}

/*
 * Initializes the pages tables for the kernel space and user space. For our current
 * new process. Append these page tables to r1_ptlist or r0_ptlist respectively.
 *
 * r1_plist : user space entries. read and write permissions
 * r0_plist : kernel space entries. read and exec permisisons
 *
 * function variables:
 *      r1_base_frame: the bottom location of the frames for the page table entires
 *      r1_top_frame: the top of the frames for the page table entires. 
 * Pages tables entries exist between base and top.
 * 
 *
 */
void init_pagetables(int r1_base_frame, int r1_top_frame)
{
	int frame_iter;
        // Build the initial page tables for Region 0 (pg 50, bullet 4)
   	// Page table entry is 32-bits wide (but does not use all 32 bits).
	// struct pte defined in hardware.h
	for (frame_iter = 0; frame_iter < physical_kernel_frames; frame_iter++) {
    	// PTE struct
    	struct pte new_pt_entry;

    	// If counter is above stack base or
  		// below the used frames make it valid
		if ((frame_iter < used_physical_kernel_frames) || (frame_iter >= (KERNEL_STACK_BASE >> PAGESHIFT)))
		   new_pt_entry.valid = (u_long) 0x1;
		else
		   new_pt_entry.valid = (u_long) 0x0; 

		// if memory is bellow kernel data start 
		// (i.e if it's kernel stach), we can read or exec,
		// if userland we can read or write (see pg 28, bullet 2) 
		if (frame_iter < (((unsigned int)kernel_data_start) >> PAGESHIFT))
      		new_pt_entry.prot = (u_long) (PROT_READ | PROT_EXEC); // exec and read protections
		else
      		new_pt_entry.prot = (u_long) (PROT_READ | PROT_WRITE); // read and write protections

		// pft (24 bits as defined on pg 28, bullet 3)
		//field contains the page frame number (the physical memory page number) 
		// of the page of physical memory to which this virtual memory page is 
		//mapped by this page table entry
		// If i get this correctly, that should just be base + ith page's address
		new_pt_entry.pfn = (u_long) ((PMEM_BASE + (frame_iter * PAGESIZE)) >> PAGESHIFT);
		
		// set up a pagetable entry to be sth like
		r0_ptlist[frame_iter] = new_pt_entry;
  
    }


	// Build the initial page tables for Region 1 (pg 50, bullet 4)
	for (frame_iter = r1_base_frame; frame_iter < r1_top_frame; frame_iter++) {
		struct pte new_pt_entry; // New pte entry

		// So far, page is invalid, has read/write protections, and no pfn
		new_pt_entry.valid = (u_long) 0x0;
		new_pt_entry.prot = (u_long) (PROT_READ | PROT_WRITE);
		new_pt_entry.pfn = (u_long) 0x0;

		// Add the page to the pagetable (accounting for 0-indexing - not sure if I need to add 1)
		r1_ptlist[frame_iter - r1_base_frame] = new_pt_entry;
	}
}

/*
 * Configures the the registers so that interrupts will go to our intterupt table, 
 * put references to the page table entries in the registers, flush the TLB so that 
 * our new page table entires will be accessable. (possibly move the flush to the 
 * init_pagetables fucntion since we need to flush before use, but makes sense to 
 * group with the other registers).
 *
 */
void config_registers()
{
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
}

/*
 * Creates out first process. the master process. pid of 0. just idles so we 
 * always have a function to switch to in the scheduler. If this process dies,
 * we halt the system as is implemented in kernel_exit.
 *
 * Function variables:
 *      user_context: pointer to a UserContext, which is the first user context,
 *              given to kernel start from the intialization of the kernel.
 *
 */
void create_idle_proc(UserContext *user_context)
{
	// Create idle process based on current user context
	// allocates within the function (see pcb.c)
	// global idle_proc pcb defined in kernel.h
           
	idle_proc = (pcb *) new_process(user_context); 

	// take two frames	
	int idle_stack_frame1 = (int) list_pop(empty_frame_list);          
	int idle_stack_frame2 = (int) list_pop(empty_frame_list); 

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
}

/*
 * Creates the second process for the kernel. Manually created since we did not
 * have fork at the time which we were doing context switching.
 *
 * TODO: replace this with somekind of fork.
 *
 * Function variables: 
 *      user_context: refers to the user contexted passed to kernel start.
 *      cmd_args: the arguements passed to the kernel start along with the 
 *              intializing program.
 *
 */
void create_init_proc(UserContext *user_context, char *cmd_args[])
{
	int i;
	 // base it of of the init (so that I don't have to re-do things)
	 pcb *init_proc = (pcb *) new_process(user_context);

	 init_proc->region0_pt = (struct pte *)malloc(KERNEL_PAGE_COUNT * sizeof(struct pte));
	 init_proc->region1_pt = (struct pte *)malloc(VREG_1_PAGE_COUNT * sizeof(struct pte));
	 bzero((char *)(init_proc->region1_pt), VREG_1_PAGE_COUNT * sizeof(struct pte));

	 // memory in region 1 should be invalid, 
	 for (i=0; i < VREG_1_PAGE_COUNT; i++) {
	 	(*(init_proc->region1_pt + i)).valid = (u_long) 0x0;
	 	(*(init_proc->region1_pt + i)).prot = (u_long) (PROT_READ | PROT_WRITE);
	 	(*(init_proc->region1_pt + i)).pfn = (u_long) 0x0;
	 }



	 for (i = 0; i < KERNEL_PAGE_COUNT; i++){
	 	(*(init_proc->region0_pt + i)).valid = (u_long) 0x1;
    	(*(init_proc->region0_pt + i)).prot = (u_long) (PROT_READ | PROT_WRITE);

    	(*(init_proc->region0_pt + i)).pfn = (u_long) (((int) list_pop(empty_frame_list) * PAGESIZE) >> PAGESHIFT);;
	 }

	// TLB flush
	WriteRegister(REG_TLB_FLUSH, TLB_FLUSH_ALL);

 	// init_proc is the first child of idle_proc!
	init_proc->parent = idle_proc;
  	idle_proc->children = init_list();
  	list_add(idle_proc->children, (void *)init_proc);

  	if (cmd_args[0] == '\0') {
  		list_add(all_procs, (void *)idle_proc);

  		curr_proc = idle_proc;


  		// copy idle's UC into the current UC
  		memcpy(user_context, idle_proc->user_context, sizeof(UserContext));
  		TracePrintf(0, "KernelStart ### End \n");

  	} else{
  		int arg_count = 0;

  		while (cmd_args[arg_count] != '\0'){
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
                TracePrintf(0, "preloadprog count:%d\n", list_count(ready_procs));
  		if ( (lpr = LoadProgram(program_name, argument_list, init_proc)) == SUCCESS ) {
  			list_add(all_procs, (void *) init_proc);
  			list_add(ready_procs, (void*) init_proc);
  			TracePrintf(3, "LoadProgram added %s on the ready list; code %d", program_name, lpr);
                        TracePrintf(0, "loadprog count:%d\n", list_count(ready_procs));
  		} else{
  			WriteRegister(REG_PTBR1, (unsigned int) &r1_ptlist);
  			WriteRegister(REG_TLB_FLUSH, TLB_FLUSH_ALL);
  			TracePrintf(3, "LoadProgram failed; code %d", lpr);
  		}
  	}
}
/* 
 * KernelStart
 *  Before allowing the execution of user processes, 
 * the KernelStart routine 
 * should perform any initialization necessary 
 * for your kernel or required by the hardware. 
 */
void KernelStart(char *cmd_args[], 
                 unsigned int phys_mem_size,
                 UserContext *user_context) 
{
	TracePrintf(0, "KernelStart ### Start\n");

	/*LOCAL VARIABLES*/

	int i; 
	int arg_count; // number of variables passed 


	// lowest frame in region 1 - see figure 2.2
	int r1_base_frame = DOWN_TO_PAGE(VMEM_1_BASE) >> PAGESHIFT;
	// highest frame in region 1 - see 3ipure 2.2
	int r1_top_frame = UP_TO_PAGE(VMEM_1_LIMIT) >> PAGESHIFT;


	
	/*GLOBAL VARIABLES*/
	init_global(phys_mem_size);

	/* PAGE TABLES */
	init_pagetables(r1_base_frame, r1_top_frame);

	/* CONFUGURE REGS */
	config_registers();


	/* 
	 * PROCESS HANDLING 
	 * IDLE
	 */
	create_idle_proc(user_context);

	/* 
	 * PROCESS HANDLING 
	 * INIT
	 */
	create_init_proc(user_context, cmd_args);

	/* 
	 * BOOKKEEPING 
	 */
	list_add(all_procs, (void *) idle_proc);
	curr_proc = idle_proc;
	// // copy idle's usercontext into the current usercontext
	memcpy(user_context, idle_proc->user_context, sizeof(UserContext));


	TracePrintf(0, "KernelStart ### End \n");
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
	TracePrintf(1, "SetKernelBrk ### Start");
	int i;

	// Check that the address is within bounds
	if ((unsigned int) addr > (unsigned int) KERNEL_STACK_BASE || \
                           addr < kernel_data_start){

                TracePrintf(3, "SetKernelBrk: ERROR ### addres: %p out of bounds", addr);
                return ERROR;
        }

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
			if ( r0_ptlist[i].valid == 0x1){
                                r0_ptlist[i].valid = (u_long) 0x1;
                        }
		}

		// from addr of pade to bottom of stack invalid
		for (i  = page_addr; i< stack_bottom; i++){
                        if ( r0_ptlist[i].valid == 0x1){
                                r0_ptlist[i].valid = (u_long) 0x0;
                        }
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
	TracePrintf(1, "SetKernelBrk ### End");
	return 0;

}

/* given idle function */
void DoIdle() {
  while (1) {
    TracePrintf(1, "\tDoIdle\n");
    Pause();
  } 
} 

/*
 * Clones the kernel context stack so that if a process does not yet have a 
 * kernel context stack, it can get one. 
 *
 * function variables:
 *      kernel_context_in: idk why we are passing this as we do not use it.
 *      current_pcb: the current process that is running.
 *      next_pcb: the next process that we want to run that does not have a
 *              kernel context stack yet.
 *
 *      TODO: figure out why we are passing kernel_context_in;
 *      
 */
void *clone_kc_stack(KernelContext *kernel_context_in, void *current_pcb, void *next_pcb)
{
	TracePrintf(2, "clone_kc_stack ### Start : my guess \n");
	//casts
	pcb *current = (pcb *) current_pcb;
	pcb *next = (pcb *) next_pcb; 
	int i;

	unsigned int dest = ((KERNEL_STACK_BASE >> PAGESHIFT) - KERNEL_PAGE_COUNT) << PAGESHIFT;
	unsigned int src = KERNEL_STACK_BASE;

	// copy current kernelto the next kc pointer
	memcpy((void *) (next->kernel_context), (void *) (current->kernel_context), sizeof(KernelContext));

	u_long bcp_valid[KERNEL_PAGE_COUNT];
	u_long bcp_pfn[KERNEL_PAGE_COUNT];

	for (i = 0; i<KERNEL_PAGE_COUNT; i++) {
		bcp_valid[i] = r0_ptlist[(KERNEL_STACK_BASE >> PAGESHIFT) - KERNEL_PAGE_COUNT + i].valid;
		bcp_pfn[i] = r0_ptlist[(KERNEL_STACK_BASE >> PAGESHIFT) - KERNEL_PAGE_COUNT + i].pfn;

		r0_ptlist[(KERNEL_STACK_BASE >> PAGESHIFT) - KERNEL_PAGE_COUNT + i].valid = (u_long) 0x1; 
		r0_ptlist[(KERNEL_STACK_BASE >> PAGESHIFT) - KERNEL_PAGE_COUNT + i].pfn = ((*(next->region0_pt + i)).pfn);
	}

	WriteRegister(REG_TLB_FLUSH, TLB_FLUSH_0);

	memcpy( (void *)dest, (void *)src, KERNEL_STACK_MAXSIZE);


	for (i = 0; i<KERNEL_PAGE_COUNT; i++) {
		r0_ptlist[(KERNEL_STACK_BASE >> PAGESHIFT) - KERNEL_PAGE_COUNT + i].valid = bcp_valid[i];
		r0_ptlist[(KERNEL_STACK_BASE >> PAGESHIFT) - KERNEL_PAGE_COUNT + i].pfn = bcp_pfn[i];
	}

	WriteRegister(REG_TLB_FLUSH, TLB_FLUSH_0);

	TracePrintf(2, "clone_kc_stack ### END \n");
    return next->kernel_context;
}


/* magic function from 5.2 */ 
/*
 * copies the kernel context into the current pcb.
 * creates a new kernel context stack if the next_pcb does not have one.
 * function variables:
 *      kernel_context_in: the current kernel context.
 *      current_pcb: the current process that is running, we save the kc into it.
 *      next_pcb: the next process that we want to run that does not have a
 *              kernel context stack yet.
 */
KernelContext *MyKCS(KernelContext *kernel_context_in, void *current_pcb, void *next_pcb)
{
	TracePrintf(2, "MyKCS ### Start : my guess \n");
	//casts
	pcb *current = (pcb *) current_pcb;
	pcb *next = (pcb *) next_pcb; 

	// save current kc
	memcpy( (void *) (current->kernel_context), (void *) kernel_context_in, sizeof(KernelContext));

	// bootstrap the kernel if we are starting
	if (next->has_kc == 0){
		clone_kc_stack(kernel_context_in, current_pcb, next_pcb);
		next->has_kc = 1;
		TracePrintf(3, "Copied idle's into next\n"); 
	}

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
    TracePrintf(2, "MyKCS ### End : my guess \n");
    return next->kernel_context;

}

/* switch to a new process - makes life easier for syscalls */
/*
 * Essentially proactively calling the scheduler when we need to switch processes
 * in a syscall. 
 *
 * function variables:
 *      user_context: the current context in userland
 *      repeat_bool: 0 if we do not need to add the current process back into the 
 *              shceduler, 1 if we do.
 */
int goto_next_process(UserContext *user_context, int repeat_bool)
{
	TracePrintf(1, "goto_next_process ### Start \n");
        
        if (list_count(ready_procs) <= 0){
                TracePrintf(0, "%d\n", list_count(ready_procs));
                return 0;
        }

        Node *temp = ready_procs->head;
        pcb *p;
        TracePrintf(3, "READY IDS\n");
        while(temp != NULL){
                p = (pcb*) temp->data;
                TracePrintf(3, "proc id: %d\n", p->process_id);
                temp = temp->next;
        }

        temp = all_procs->head;
        TracePrintf(3, "ALL IDS\n");
        while(temp != NULL){
                p = (pcb*) temp->data;
                TracePrintf(3, "proc id: %d\n", p->process_id);
                temp = temp->next;
        }


	if (repeat_bool) {
		list_add(ready_procs, (void *) curr_proc);
	}
        TracePrintf(6, "goto_next_process: about to pop a process\n");
	// context switching
	pcb *next_proc = (pcb *) list_pop(ready_procs);
        TracePrintf(6, "goto_next_process: popped a process, switching to %d \n", next_proc->process_id);

	if (context_switch(curr_proc, next_proc, user_context) != 0){
		return FAILURE;
	}

	TracePrintf(1, "goto_next_process ### End \n");

	return 0;
	
}

/*
 * Switches the context from the current pcb to the next pcb.
 */
int context_switch(pcb *current, pcb *next, UserContext *user_context)
{
	TracePrintf(2, "ContextSwitch ### Start \n");
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
	TracePrintf(2, "ContextSwitch ### End \n");
	return r;
}	


void scheduler(void)
{
	TracePrintf(2, "Scheduler ### Start \n");

	unsigned int i;
	int retval;

	pcb *next_pcb;
	pcb *curr_pcb = curr_proc;

	if (list_count(ready_procs) > 0){
		next_pcb = list_pop(ready_procs);
		curr_proc = next_pcb;

		retval = KernelContextSwitch(MyKCS, (void *) curr_pcb, (void *) next_pcb);
	}

	if (retval != 0){
		TracePrintf(1, "Scheduler ### ERROR KCS FAILED");
		exit(FAILURE);
	}

	TracePrintf(2, "Scheduler ### End \n");

}


/*=============================================================================
 |   Assignment:  Yalnix Kernel Header files
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
 | Description:  Includes and definitions for the magic that happens in the kernel
 |
 | Known Bugs:  it doesn't really work
 |
 *===========================================================================*/

#ifndef _KERNEL_H_
#define _KERNEL_H_

/*
 * System Includes From Sean's Manual
 */
#include <hardware.h>
#include <yalnix.h>


/*
 * Local Includes
 */
#include "list.h"
#include "pcb.h"
#include "interupts.h"


/*
 * Cool constants
 */
#define VREG_1_PAGE_COUNT  (((VMEM_1_LIMIT - VMEM_1_BASE) / PAGESIZE))
#define VREG_0_PAGE_COUNT  (((VMEM_0_LIMIT - VMEM_0_BASE) / PAGESIZE))
#define KERNEL_PAGE_COUNT  (KERNEL_STACK_MAXSIZE / PAGESIZE)
#define RESERVED_KERNEL_PAGE ((VMEM_0_LIMIT - KERNEL_STACK_MAXSIZE - PAGESIZE)>>PAGESHIFT)



/*
 * Global variables
 */

// SetKernelData - see kernel.c ln 18:29
void *kernel_data_start;
void *kernel_data_end;


// memory management - see kernel.c ln 59
void *kernel_brk;

// phisical frames tracking - see kernel.c ln 55:6-
unsigned int used_physical_kernel_frames;
unsigned int physical_kernel_frames;
unsigned int total_physical_frames;


List *empty_frame_list;


// process ttracking - see kernel.c ln 60
unsigned int available_process_id;



// process tracking lists -
List *ready_procs;
List *blocked_procs;
List *all_procs;
List *zombie_procs;


// Page table List for both regions (statically defined)
// both pte struct and constants defined in hardware.h
struct pte r0_ptlist[VREG_0_PAGE_COUNT];                                     
struct pte r1_ptlist[VREG_1_PAGE_COUNT]; 



// List *locks;
// List *cvars;
// List *pipes;
// List *ttys;


// processes
pcb *curr_proc;
pcb *idle_proc;






/* 
 * Public Facing Function Calls 
 */
 
void SetKernelData(void * _KernelDataStart, void *_KernelDataEnd);
void KernelStart(char *cmd_args[], 
                 unsigned int phys_mem_size,
                 UserContext *user_context);
void init_global(int);
void init_pagetables(int, int);
void config_registers();
void create_idle_proc(UserContext *user_context);
void create_init_proc(UserContext *user_context, char *cmd_args[]);

int SetKernelBrk(void * addr);
void DoIdle();
int goto_next_process(UserContext *user_context, int repeat_bool);
int context_switch(pcb *current, pcb *next, UserContext *user_context);
void scheduler(void);
#endif

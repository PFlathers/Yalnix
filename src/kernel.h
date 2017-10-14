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
#include "listlist.h"
#include "pcb.h"


// globals

// SetKernelData - see kernel.c ln 18:29
void *kernel_data_start;
void *kernel_data_end;


// memory management - see kernel.c ln 59
void *kernel_brk;

// phisical frames tracking - see kernel.c ln 55:6-
unsigned int used_physical_kernel_frames;
unsigned int physical_kernel_frames;
unsigned int total_phisical_frames;

list empty_frame_list;


// process ttracking - see kernel.c ln 60
unsigned int available_process_id;


// process tracking lists -
list *ready_procs;
list *blocked_procs;
list *all_procs;
list *zombie_procs;


// Page table list for both regions (statically defined)
// both pte struct and constants defined in hardware.h
struct pte r0_ptlist[VMEM_0_PAGE_COUNT];                                     
struct pte r1_ptlist[VMEM_1_PAGE_COUNT]; 


list *locks;
list *cvars;
list *pipes;
list *ttys;








/* Public Facing Function Calls */


#endif
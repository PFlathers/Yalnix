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
int next_resource_id;
list *locks;
list *cvars;
list *pipes;
list *ttys;



// SetKernelData
void *kernel_data_start;
void *kernel_data_end;



/* Public Facing Function Calls */


#endif
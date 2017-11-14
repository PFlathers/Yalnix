#ifndef _UTILS_
#define _UTILS_
#include "cvar.h"
#include "pipe.h"
#include "lock.h"

//finds and returns cvar from passed id
Cvar *kernel_findCvar(int cvar_id);

//finds and returns lock from passed id
Lock *kernel_findLock(int lock_id);

//finds and returns pipe from passed id
Pipe *kernel_findPipe(int lock_id);

/* WARNING!!! THIS IS A DANGEROUS FUNCTION!
 * trashes, frees, and recycles the page tables from the passed process.
 */
void free_pagetables(pcb* myproc);

int check_pointer_range(u_long ptr);
int check_pointer_valid(u_long ptr);
int check_pointer_read(u_long ptr);
int check_pointer_write(u_long ptr);
int is_rw(u_long ptr);
int check_string_validity(u_long ptr, int len);
#endif

#include "pcb.h"
#include "lock.h"
#include "cvar.h"
#include "pipe.h"
#include "list.h"
#include "kernel.h"
#include "syscalls.h"

/* Finds and returns cvar of associated id from the global cvar list
 */
Cvar *kernel_findCvar(int cvar_id)
{
        Node *temp = cvars->head;
        while (temp != NULL){
                if ( ((Cvar*)temp->data)->id == cvar_id)
                        break;
                temp = temp->next;
        }
        if(temp == NULL){
                TracePrintf(5, "Cvar does not exist\n");
                return NULL;
        } 
        return (Cvar*) temp->data;
}

/* Finds and returns lock of associated id from the global lock list
 */
Lock *kernel_findLock(int lock_id)
{
        Node *temp = locks->head;
        while (temp != NULL){
                if ( ((Lock*)temp->data)->id == lock_id)
                        break;
                temp = temp->next;
        }
        if(temp == NULL){
                TracePrintf(5, "Lock does not exist\n");
                return NULL;
        } 
        return (Lock*) temp->data;
}

/* Finds and returns pipe of associated id from the global pipe list
 */
Pipe *kernel_findPipe(int lock_id)
{
        Node *temp = pipes->head;
        while (temp != NULL){
                if ( ((Pipe*)temp->data)->id == lock_id)
                        break;
                temp = temp->next;
        }
        if(temp == NULL){
                TracePrintf(5, "Pipe does not exist\n");
                return NULL;
        } 
        return (Pipe*) temp->data;
}

/* WARNING!!! BE CAREFUL WITH THIS FUCNTION!
 * Trashes and frees the memory assoctiated with the process.
 * Moves the now free pagetable memory back in to the empty page list
 *
 * If you come accross an erorr where a procecss id becomes an address, its
 * probably from this.
 */
void free_pagetables(pcb* myproc)
{
        int i;
        int trash_pfn;

         
        //Recycle the kernel page tables
        for (i = 0; i < KERNEL_PAGE_COUNT; i ++){
                if( (*(myproc->region0_pt + i)).valid == 0x1){
                        trash_pfn = FRAME_TO_PAGE( (*(myproc->region0_pt + i)).pfn );
                        list_add(empty_frame_list, (void *) trash_pfn);

                        (*(myproc->region0_pt + i)).valid = (u_long) 0x0;
                        (*(myproc->region0_pt + i)).pfn = (u_long) 0x0;
                }
        }
        
        //Recycle the userland page tables
        for (i = 0; i < VREG_1_PAGE_COUNT; i++){
                if( (*(myproc->region1_pt + i)).valid == 0x1 ){
                        trash_pfn = FRAME_TO_PAGE( (*(myproc->region0_pt + i)).pfn );
                        list_add(empty_frame_list, (void*) trash_pfn);

                        (*(myproc->region1_pt + i)).pfn = (u_long) 0x0;
                        (*(myproc->region1_pt + i)).valid= (u_long) 0x0;
                }
        }
        
        free(myproc->user_context);
        free(myproc->kernel_context);

        //Dont forget to flush.
        WriteRegister(REG_TLB_FLUSH, TLB_FLUSH_ALL);
}

/* Checks whether or not pointer is in range of the valid page table entires
 * for the process
 */
int check_pointer_range(u_long ptr) 
{                                                         
        // if argument is withing outside the bounds return 1, else return 0
        if((unsigned int) ptr < VMEM_1_BASE || (unsigned int) ptr > VMEM_1_LIMIT){
                return 1;
        } else {
                return 0;
        }                                                                            
}   


/* Checks whether or not the point is to valid memory for the process
 */
int check_pointer_valid(u_long ptr)
{                                                          
        struct pte *ptr_pte;
        ptr_pte = curr_proc->region1_pt + (ptr >> PAGESHIFT) - 128;

        if (ptr_pte->valid != (u_long) 0x1)
                return 1;
        else
                return 0;
}                                                                                  

/* Checks whether there are read permission on the memory the pointer is
 * pointing to
 */
int check_pointer_read(u_long ptr) 
{                                                          
        struct pte *ptr_pte;
        ptr_pte = curr_proc->region1_pt + (ptr >> PAGESHIFT) - 128;

        if (ptr_pte->prot | (u_long) PROT_READ)
                return 0;
        else
                return 1;
}                                                                                  
                                                                                    
/* Checks whether there are write permission on the memory the pointer is
 * pointing to
 */
int check_pointer_write(u_long ptr) 
{                                                         
        struct pte *ptr_pte;
        ptr_pte = curr_proc->region1_pt + (ptr >> PAGESHIFT) - 128;

        if (ptr_pte->prot | (u_long) PROT_WRITE)
                return 0;
        else
                return 1;

}

/* Checks both read and write
 */
int is_rw(u_long ptr)
{
        return (check_pointer_write(ptr) || check_pointer_read(ptr));
} 

/* Checks all the permissions for a String
 */
int check_string_validity(u_long ptr, int len) 
{
        int i;

        for (i = 0; i < (len / PAGESIZE); i++) {
                if (check_pointer_range(ptr + (i * PAGESIZE)) ||
                        check_pointer_valid(ptr + (i * PAGESIZE)) ||
                        is_rw(ptr + (i * PAGESIZE))
                )
                return 1;
        }

        return 0;
}         

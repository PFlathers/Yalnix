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


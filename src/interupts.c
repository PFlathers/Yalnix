#include <stdlib.h>
#include <hardware.h>
#include <string.h>
#include "pcb.h"
#include "kernel.h"
#include "syscalls.h"
#include "globals.h"

/* local utilities */
int check_pointer_range(u_long ptr);
int check_pointer_write(u_long ptr) ;                                                         

int check_pointer_valid(u_long ptr) ;                                                    



/* actual interupts */

void trapKernel(UserContext *uc)
{
  int clock_ticks;
  int retval = 0;
  int exit_code;
  void *addr;

  // weee - switching
  switch(uc->code) { 
      case YALNIX_EXEC:
        TracePrintf(3, "trapKernel: YALNIX_EXEC\n");

        if ( check_pointer_range(uc->regs[0]) \
          || check_pointer_range(uc->regs[1]) ){

          TracePrintf(3, "trapKernel: error in EXEC, out of range\n");
          retval = ERROR;
          break;
        }

        if ( check_pointer_valid(uc->regs[0]) \
          || check_pointer_valid(uc->regs[1]) ){

          TracePrintf(3, "trapKernel: error in EXEC, out of range\n");
          retval = ERROR;
          break;
        }
        if ( is_rw(uc->regs[0]) \
          || is_rw(uc->regs[1]) ){

          TracePrintf(3, "trapKernel: error in EXEC, out of range\n");
          retval = ERROR;
          break;
        }

        char *fname_p = (char *) uc->regs[0];
        char **arg_p = (char **) uc->regs[1];
        int file_size = (strlen(fname_p) + 1) * sizeof(char);

        char *filename = (char *) malloc(file_size);
        ALLOC_CHECK(filename, "exec");
        memcpy((void*) filename, (void*) fname_p, file_size);
        TracePrintf(6, "trapKernel: calling exec on %s\n", filename);
        TracePrintf(3, "tk:%s\n", arg_p[0]);

        kernel_Exec(uc, filename, arg_p); 
        break;

      case YALNIX_DELAY:
        TracePrintf(3, "trapKernel: YALNIX_DELAY \n");
         clock_ticks = (int) uc->regs[0];
        int retval = kernel_Delay(uc, clock_ticks);
        break;

      case YALNIX_FORK:
        TracePrintf(3, "trapKernel: YALNIX_FORK\n");
        retval = kernel_Fork(uc);
          TracePrintf(0, "retval: %d\n", retval);
        break;

      case YALNIX_EXIT:
        TracePrintf(3, "trapKernel: YALNIX_EXIT\n");
      	int status = (int) uc->regs[0];
      	kernel_Exit(status, uc);
        break;

      case YALNIX_WAIT:
        TracePrintf(3, "trapKernel: YALNIX_WAIT\n");
        // check if in range
        if ( check_pointer_range(uc->regs[0]) ){
          TracePrintf(3, "trapKernel: error in WAIT, out of range\n");
          retval = ERROR;
          break;
        }
        // check if pages are valid
        if ( check_pointer_valid(uc->regs[0]) ){
          TracePrintf(3, "trapKernel: error in WAIT, out of range\n");
          retval = ERROR;
          break;
        }
        // check if RW
        if ( is_rw(uc->regs[0]) ){
          TracePrintf(3, "trapKernel: error in WAIT, out of range\n");
          retval = ERROR;
          break;
        }

        int *status_pt = (int *) uc->regs[0];
        retval = kernel_Wait(status_pt, uc);
        break;

      case YALNIX_GETPID:
        TracePrintf(3, "trapKernel: YALNIX_GETPID\n");
        retval = kernel_GetPid(uc);
        break;

      case YALNIX_BRK:
        // check if in range
        if ( check_pointer_range(uc->regs[0]) ){
          TracePrintf(3, "trapKernel: error in WAIT, out of range\n");
          retval = ERROR;
          break;
        }
        // check if pages are valid
        if ( check_pointer_valid(uc->regs[0]) ){
          TracePrintf(3, "trapKernel: error in WAIT, out of range\n");
          retval = ERROR;
          break;
        }
        // check if RW
        if ( is_rw(uc->regs[0]) ){
          TracePrintf(3, "trapKernel: error in WAIT, out of range\n");
          retval = ERROR;
          break;
        }
        addr = (void *) uc->regs[0];
        retval = kernel_Brk(addr);
        break;

      default:
        TracePrintf(3, "Unrecognized syscall: %d\n", uc->code);
        break;
    }

  // set return value  
  uc->regs[0] = retval;
}

void trapClock(UserContext *uc)
{
  TracePrintf(1, "trapClock ### start \n");

        Node *temp = all_procs->head;
        pcb *p;

        TracePrintf(3, "ALL PROCS\n");
        while (temp!=NULL){
                p = (pcb*) temp->data;
                TracePrintf(3, "%d\n", p->process_id);
                temp = temp->next;
        }

        TracePrintf(3, "READY PROCS\n");
        temp = ready_procs->head;
        while (temp!=NULL){
                p = (pcb*) temp->data;
                TracePrintf(3, "%d\n", p->process_id);
                temp = temp->next;
        }

        TracePrintf(3, "ZOMBIE PROCS\n");
        temp = zombie_procs->head;
        while (temp!=NULL){
                p = (pcb*) temp->data;
                TracePrintf(3, "%d\n", p->process_id);
                temp = temp->next;
        }



  if (list_count(blocked_procs) > 0) {
    Node *curr_on_delay = (Node *) blocked_procs->head;

    while(curr_on_delay->next != NULL){
      pcb *delay_pcb = (pcb *)curr_on_delay->data;
      curr_on_delay = curr_on_delay->next;

      if (check_block_status(delay_pcb->block) == 0){
        list_remove(blocked_procs, delay_pcb);
        list_add(ready_procs, delay_pcb);
      }
    }

    // handle last idem edge case (reason why it didn't work in chckp 3)
    if (check_block_status(((pcb*)curr_on_delay->data)->block) == 0) {
          // Remove it from the blocked queue and add it to ready queue
          pcb *delay_pcb = (pcb *) curr_on_delay->data;
          list_remove(blocked_procs, delay_pcb);
          list_add(ready_procs, delay_pcb);
    }
  }
  TracePrintf(3, "trapClock: proc is Id: %d\n", curr_proc->process_id);
  if (list_count(ready_procs) > 0){
    TracePrintf(3, "trapClock: Switching processes\n");
    int rc = goto_next_process(uc, 1);
    TracePrintf(6, "trapClock: done switching processes\n");
  }
  else{
    TracePrintf(3, "trapClock: no process to switch to gonna keep going");
  }



  TracePrintf(1, "trapClock ### end \n");
}

void trapIllegal(UserContext *uc)
{
}

void trapMemory(UserContext *uc)
{
}

void trapMath(UserContext *uc)
{
}

void trapTTYReceive(UserContext *uc)
{
}

void trapTTYTransmit(UserContext *uc)
{
}

void trapname1(UserContext *uc){}
void trapname2(UserContext *uc){}
void trapname3(UserContext *uc){}
void trapname4(UserContext *uc){}
void trapname5(UserContext *uc){}
void trapname6(UserContext *uc){}
void trapname7(UserContext *uc){}
void trapname8(UserContext *uc){}
void trapname9(UserContext *uc){}



/* =============== UTILITIES ========================== */



int check_pointer_range(u_long ptr) 
{                                                         
  // if argument is withing outside the bounds return 1, else return 0
  if((unsigned int) ptr < VMEM_1_BASE || (unsigned int) ptr > VMEM_1_LIMIT){
    return 1;
  }                                                                      
  else {
    return 0;
  }                                                                            
}   


int check_pointer_valid(u_long ptr)
{                                                          
  struct pte *ptr_pte;
  ptr_pte = curr_proc->region1_pt + (ptr >> PAGESHIFT) - 128;

  if (ptr_pte->valid != (u_long) 0x1)
    return 1;
  else
    return 0;
};                                                                                  
                                                                                    
int check_pointer_read(u_long ptr) 
{                                                          
  struct pte *ptr_pte;
  ptr_pte = curr_proc->region1_pt + (ptr >> PAGESHIFT) - 128;

  if (ptr_pte->prot | (u_long) PROT_READ)
    return 0;
  else
    return 1;
};                                                                                  
                                                                                    
int check_pointer_write(u_long ptr) 
{                                                         
  struct pte *ptr_pte;
  ptr_pte = curr_proc->region1_pt + (ptr >> PAGESHIFT) - 128;

  if (ptr_pte->prot | (u_long) PROT_WRITE)
    return 0;
  else
    return 1;

};

int is_rw(u_long ptr)
{
  return (check_pointer_write(ptr) || check_pointer_read(ptr));
}              
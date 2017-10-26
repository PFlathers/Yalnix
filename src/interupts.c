#include <stdlib.h>
#include <hardware.h>
#include "pcb.h"
#include "kernel.h"
#include "syscalls.h"

//Prototypes of kernel traps.
//This will be flushed out later.

void trapKernel(UserContext *uc)
{
  int clock_ticks;
  int retval;
  int exit_code;
  void *addr;

  // weee - switching
  switch(uc->code) { 
      case YALNIX_EXEC:
        TracePrintf(3, "trapKernel: YALNIX_EXEC\n");
        break;

      case YALNIX_DELAY:
        TracePrintf(3, "trapKernel: YALNIX_DELAY \n");
         clock_ticks = (int) uc->regs[0];
        int retval = kernel_Delay(uc, clock_ticks);
        break;

      case YALNIX_FORK:
        TracePrintf(3, "trapKernel: YALNIX_FORK\n");
        break;

      case YALNIX_EXIT:
        TracePrintf(3, "trapKernel: YALNIX_EXIT\n");
        break;

      case YALNIX_WAIT:
        TracePrintf(3, "trapKernel: YALNIX_WAIT\n");
        // check if in range
        // check if pages are valid
        // check if RW

        int *status_pt = (int *) uc->regs[0];
        retval = kernel_Wait(status_pt, uc);
        break;

      case YALNIX_GETPID:
        TracePrintf(3, "trapKernel: YALNIX_GETPID\n");
        retval = kernel_Getpid(uc);
        break;

      case YALNIX_BRK:
        // check if in raed
        // check if valid
        // check if RW
        addr = (void *) uc->regs[0];
        retval = Yalnix_Brk(addr);
        break;

      default:
        TracePrintf(3, "Unrecognized syscall: %d\n", uc->code);
        break;
    }
}

void trapClock(UserContext *uc)
{
  TracePrintf(1, "trapClock ### start \n");

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
    goto_next_process(uc, 1);
  }
  else{
    TracePrintf(3, "trapClock: no process to switch to gonna keep going")
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

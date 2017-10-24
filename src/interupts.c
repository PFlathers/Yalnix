#include <stdlib.h>
#include <hardware.h>
#include "pcb.h"
#include "kernel.h"
#include "syscalls.h"

//Prototypes of kernel traps.
//This will be flushed out later.

void trapKernel(UserContext *uc)
{
   TracePrintf(1, "blah \n");
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

    if (check_block_status(((pcb*)curr_on_delay->data)->block) == 0) {
          // Remove it from the blocked queue and add it to ready queue
          pcb *delay_pcb = (pcb *) curr_on_delay->data;
          list_remove(blocked_procs, delay_pcb);
          list_add(ready_procs, delay_pcb);
    }


    scheduler();
  }

  // add call to round robit


  TracePrintf(3, "Proc Id: %d\n", curr_proc->process_id);
  if (list_count(ready_procs) > 0){
    TracePrintf(3, "Switching processes\n");
    goto_next_process(uc, 1);
  }
  else{
    TracePrintf(3, "No process to switch to or in proc 1- gonna keep going\n");
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

#include <stdlib.h>
#include <hardware.h>
#include <yalnix.h>
#include <string.h>
#include "pcb.h"
#include "kernel.h"
#include "syscalls.h"
#include "globals.h"
#include "tty.h"

/* local utilities */
int check_pointer_range(u_long ptr);
int check_pointer_write(u_long ptr);                                                         
int check_string_validity(u_long ptr, int len);
int check_pointer_valid(u_long ptr);                                                    



/* actual interupts */

void trapKernel(UserContext *uc)
{
  int clock_ticks;
  int retval = 0;
  int exit_code;
  void *addr;
  int len;
  int tty_id;
  void *buf;

  // weee - switching
  switch(uc->code) { 
      case YALNIX_EXEC:
        TracePrintf(3, "trapKernel: YALNIX_EXEC\n");

        if ( check_pointer_range(uc->regs[0]) 
          || check_pointer_range(uc->regs[1]) ){

          TracePrintf(3, "trapKernel: error in EXEC, out of range\n");
          retval = ERROR;
          break;
        }

        if ( check_pointer_valid(uc->regs[0])
          || check_pointer_valid(uc->regs[1]) ){

          TracePrintf(3, "trapKernel: error in EXEC, invalid\n");
          retval = ERROR;
          break;
        }
        if ( is_rw(uc->regs[0]) \
          || is_rw(uc->regs[1]) ){

          TracePrintf(3, "trapKernel: error in EXEC, not rw\n");
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
        retval = kernel_Delay(uc, clock_ticks);
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
         //check if in range
        if ( check_pointer_range(uc->regs[0]) ){
          TracePrintf(3, "trapKernel: error in WAIT, pointer out of range\n");
          retval = ERROR;
          break;
        }
        // check if pages are valid
        if ( check_pointer_valid(uc->regs[0]) ){
          TracePrintf(3, "trapKernel: error in WAIT, page not valid \n");
          retval = ERROR;
          break;
        }
        // check if RW
        if ( is_rw(uc->regs[0]) ){
          TracePrintf(3, "trapKernel: error in WAIT, page not rw\n");
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

      case YALNIX_PIPE_INIT:
        // check if in range
        if ( check_pointer_range(uc->regs[0]) ){
          TracePrintf(3, "trapKernel: error in PipeInit, pointer out of range\n");
          retval = ERROR;
          break;
        }
        // check if pages are valid
        if ( check_pointer_valid(uc->regs[0]) ){
          TracePrintf(3, "trapKernel: error in PipeInit, pointer address not valid \n");
          retval = ERROR;
          break;
        }
        // check if RW
        if ( is_rw(uc->regs[0]) ){
          TracePrintf(3, "trapKernel: error in PipeInit, address not rw\n");
          retval = ERROR;
          break;
        }

        retval = PipeInit((int*) uc->regs[0]);
        break;



      case YALNIX_PIPE_READ:
        if( check_string_validity(uc->regs[1], uc->regs[2]) ){
          retval = ERROR;
          break;
        }
        memcpy((void*) curr_proc->user_context, (void*) uc, sizeof(UserContext));
        retval = kernel_PipeRead((int) uc->regs[0], (void *) uc->regs[1], (int) uc->regs[2], uc);
        break;

      case YALNIX_PIPE_WRITE:
        if( check_string_validity(uc->regs[1], uc->regs[2]) ){
          retval = ERROR;
          break;
        }
        retval = kernel_PipeWrite((int) uc->regs[0], (void * ) uc->regs[1], (int) uc->regs[2]);
        break;

      case YALNIX_BRK:
        // check if in range
        
        if ( check_pointer_range(uc->regs[0]) ){
                TracePrintf(0, "trapKernel: brk to set is: %d\n", (int)uc->regs[0]);
          TracePrintf(3, "trapKernel: error in Brk, pointer out of range\n");
          retval = ERROR;
          break;
        }
        // check if pages are valid
       
        if ( check_pointer_valid(uc->regs[0]) ){
          TracePrintf(3, "trapKernel: error in Brk, pointer address not valid\n");
          retval = ERROR;
          break;
        }
        // check if RW
        if ( is_rw(uc->regs[0]) ){
          TracePrintf(3, "trapKernel: error in Brk, address not rw \n");
          retval = ERROR;
          break;
        }
        addr = (void *) uc->regs[0];
        retval = kernel_Brk(addr);
        break;

       case YALNIX_TTY_WRITE:
            if ( check_string_validity(uc->regs[1], uc->regs[2]) ){
             TracePrintf(3, "trapTTYWRITE: error in sting, out of range\n");
             retval = ERROR;
             break;
            }

         tty_id = (int) uc->regs[0];
         buf = (void *) uc->regs[1];
         len = (int) uc->regs[2];
         retval = kernel_TtyWrite(tty_id, buf, len);
         break;

       case YALNIX_TTY_READ:
            if ( check_string_validity(uc->regs[1], uc->regs[2]) ){
             TracePrintf(3, "trapTTYWRITE: error in sting, out of range\n");
             retval = ERROR;
             break;
            }
         //check string validity in uc->regs

         tty_id = (int) uc->regs[0];
         buf = (void *) uc->regs[1];
         len = (int) uc->regs[2];
         retval = kernel_TtyRead(tty_id, buf, len);
         break;
        case YALNIX_LOCK_INIT:
                   //check if in range
                if ( check_pointer_range(uc->regs[0]) ){
                  TracePrintf(3, "trapKernel: error in YALNIX_LOCK_INIT, pointer out of range\n");
                  retval = ERROR;
                  break;
                }
                // check if pages are valid
                if ( check_pointer_valid(uc->regs[0]) ){
                  TracePrintf(3, "trapKernel: error in YALNIX_LOCK_INIT, page not valid \n");
                  retval = ERROR;
                  break;
                }
                // check if RW
                if ( is_rw(uc->regs[0]) ){
                  TracePrintf(3, "trapKernel: error in YALNIX_LOCK_INIT, page not rw\n");
                  retval = ERROR;
                  break;
                }
                retval = kernel_LockInit((int*) uc->regs[0]);
                break;
        case YALNIX_LOCK_ACQUIRE:
                retval = kernel_Acquire((int) uc->regs[0]);
                break;
        case YALNIX_LOCK_RELEASE:
                retval = kernel_Release((int) uc->regs[0]);
                break;

        case YALNIX_CVAR_INIT:
          retval = kernel_CvarInit((int*)uc->regs[0]);
          break;

        case YALNIX_CVAR_SIGNAL:
          retval = kernel_CvarSignal((int)uc->regs[0]);
          break;

        case YALNIX_CVAR_BROADCAST:
          retval = kernel_CvarBroadcast((int)uc->regs[0]);
          break;
          
        case YALNIX_CVAR_WAIT:
          retval = kernel_CvarWait((int)uc->regs[0], (int)uc->regs[1]);
          break;


        case YALNIX_RECLAIM:
          retval = kernel_Reclaim((int)uc->regs[0]);
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
        memcpy((void*) curr_proc->user_context, (void*) uc, sizeof(UserContext));

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
        TracePrintf(1, "trapIllegal ### start: now exiting \n");
        TracePrintf(6, "\t Illegal pid = %d \n \t status: %d",
            curr_proc->process_id, uc->code);
      	int status = -1;
      	kernel_Exit(status, uc);
        uc->regs[0] = status;
}

void trapMemory(UserContext *uc)
{
        TracePrintf(0, "Illegal Memory code %d\n", uc->code);
      	int status = -1;

        if (uc->code == YALNIX_ACCERR) {
          TracePrintf(6, "\t Process had access error at %p \n", uc->code);
        }
        else if (uc->code == YALNIX_MAPERR) {
          TracePrintf(6, "\t Process had a mapping error \n");
          if ((unsigned int)uc->addr < curr_proc->brk_address || 
                (unsigned int)uc->addr > (unsigned int) uc->sp) {
                  TracePrintf(6, "\t exiting, real mapping err so sorry \n");
                  int brk_over = ((unsigned int)uc->addr < curr_proc->brk_address);
                  TracePrintf(6, "\t error: uc->addr %p,  brk_address %d \n", (unsigned int)uc->addr, curr_proc->brk_address);
                  kernel_Exit(status, uc);
                  uc->regs[0] = status;
          }
        }

      unsigned int addr_page = DOWN_TO_PAGE(uc->addr) >> PAGESHIFT;
      unsigned int stack_top_page = VMEM_1_LIMIT >> PAGESHIFT;
      unsigned int usr_brk_page = UP_TO_PAGE(curr_proc->brk_address) >> PAGESHIFT;
      unsigned int usr_heap_bottom = curr_proc->heap_start_pg;
      
      int i; 

      // check if the requested adreess would mess up the heap,
      // i.e there should be at least one page to make it work
      // two not to mess it up
      if (addr_page - usr_brk_page <= 2) {
            TracePrintf(6, "Illegal Memory: process out of memory \n");
            kernel_Exit(status, uc);
            uc->regs[0] = status;
      }   

      unsigned int vm_pg_start = addr_page - 128;
      unsigned int vm_pg_end = stack_top_page - 128;
      struct pte *temp;
      for (i = vm_pg_start; i<= vm_pg_end; i++){
          temp = curr_proc->region1_pt + i;

          // no page allocated for region 1 pt
          if (temp->valid == 0x0) {
              if (list_count(empty_frame_list) < 1) {
                  TracePrintf(6, "Illegal Memory: not enough frames to grow stack \n");
                  kernel_Exit(status, uc);
                  uc->regs[0] = status;
              }

              // set page to valid
              temp->valid = 0x1;
              temp->prot = (PROT_READ | PROT_WRITE);

              // map  aframe to it
              int pfn = (int) list_pop(empty_frame_list);
              temp->pfn = (u_long) ( (pfn * PAGESIZE) >> PAGESHIFT);

          }
      }
      // flush me baby flush me baby
      WriteRegister(REG_TLB_FLUSH, TLB_FLUSH_ALL);
      TracePrintf(0, "Illegal Memory: Holly shit we survived%d\n");

      	
}

void trapMath(UserContext *uc)
{
        TracePrintf(0, "trapMath ### start: now exiting\n");
      	int status = -1;
      	kernel_Exit(status, uc);
        uc->regs[0] = status;
}

void trapTTYReceive(UserContext *uc)
{

        TracePrintf(0, "trapTTYReceive ### start\n");

        // find the tty by uc->code
        int tty_id = uc->code;
        TTY *tty = NULL;
        Node * node =  ttys->head;
        while (node->next != NULL){
                if ( ((TTY*)(node->data))->tty_id == tty_id){
                        tty = (TTY *) node->data;
                        break;
                }
                node = node->next;
        }
        //edge case
        if ( ((TTY*)(node->data))->tty_id == tty_id) {
                //...
                tty = ((TTY*)(node->next->data));
                // tty->next->data;
                //...
        }

        if (tty == NULL) {
                TracePrintf(6, "TtyWrite: ERROR; tty %d out of bounds \
                                - should not happen \n", tty_id);
                return;// ERROR;
        }

        //allocate new buffer
        Buffer *new = (Buffer *) malloc(sizeof(Buffer));
        new->buf = (char *) malloc(TERMINAL_MAX_LINE);
        // call TtyRecieve
        new->len = TtyReceive(tty->tty_id, new->buf, TERMINAL_MAX_LINE);

        

        // add the created buffer on the list of buffers
        // for that tty
        list_add(tty->buffers, new);

        

        // if there is anyone on the to_read list, 
        // put it back on the ready list
        int len = new->len;
        while ( (list_count(tty->to_read) > 0) && (len > 0) ) {
                pcb *waiter = list_pop(tty->to_read);
                list_add(ready_procs, waiter);
                len = len - waiter->read_length;
        }

        TracePrintf(0, "trapTTYReceive ### end me baby\n");

}

void trapTTYTransmit(UserContext *uc)
{
        TracePrintf(0, "trapTTYTransmit ### start\n");
        // find the tty by uc->code
        int tty_id = uc->code;
        TTY *tty = NULL;
        Node * node =  ttys->head;
        while (node/*->next*/ != NULL){
                if ( ((TTY*)(node->data))->tty_id == tty_id){
                        tty = (TTY *) node->data;
                        break;
                }
                node = node->next;
        }
        //edge case
        /*
        if ( ((TTY*)(node->next->data))->tty_id == tty_id) {
                tty = ((TTY*)(node->next->data));
        }
*/
        if (tty == NULL) {
                TracePrintf(6, "TtyWrite: ERROR; tty %d out of bounds \
                                - should not happen \n", tty_id);
                return ;//ERROR;
        }
        if(tty->to_write || tty->to_write->count == 0)
        {
                TracePrintf(3, "Bruno's TracePrint here\n");
        }else{
                pcb *writer = list_pop(tty->to_write);
                // check if we need to write more then once,
                // otherwise call TtyTransmit
                int remains = writer->buffer->len - TERMINAL_MAX_LINE;
                if (remains > 0 ) {
                        writer->buffer->len = remains;
                        writer->buffer->buf = writer->buffer + TERMINAL_MAX_LINE;

                        list_add(tty->to_write, (void *) writer);

                        if (remains > TERMINAL_MAX_LINE) {
                                TtyTransmit(tty->tty_id, writer->buffer->buf, TERMINAL_MAX_LINE);
                        }
                        else {
                                TtyTransmit(tty->tty_id, writer->buffer->buf, writer->buffer->len);
                        }
                }
                else {
                        // check if there are waiting process, and call
                        // the transmit for them (see where I break it up
                        // in the syscall)

                        list_add(ready_procs, writer);

                        if (list_count(tty->to_write) > 0) {
                                pcb *next = list_pop(tty->to_write);
                                list_add(tty->to_write, (void *) next);

                                int next_len = (int) next->buffer->len;
                                if (next_len > TERMINAL_MAX_LINE){
                                        TtyTransmit(tty->tty_id, next->buffer->buf, TERMINAL_MAX_LINE);
                                }
                                else {
                                        TtyTransmit(tty->tty_id, next->buffer->buf, next_len);
                                }
                        }

                }
        }
        TracePrintf(0, "trapTTYTransmit ### end\n");

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


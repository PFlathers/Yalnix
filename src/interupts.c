#include <stdlib.h>
#include <hardware.h>
#include <yalnix.h>
#include <string.h>
#include "pcb.h"
#include "kernel.h"
#include "syscalls.h"
#include "globals.h"
#include "tty.h"
#include "kernel_utils.h"

/* local utilities for checking validity of passed pointers*/
int check_pointer_range(u_long ptr);
int check_pointer_write(u_long ptr);                                                         
int check_string_validity(u_long ptr, int len);
int check_pointer_valid(u_long ptr);                                                    



/* actual interupts */

//Handles all kernel interrupts
void trapKernel(UserContext *uc)
{
  int clock_ticks;

  /*This value that will be returned to userland
   *Warning: Do not redefine while switching traps
   */
  int retval = 0;
  int len;
  int tty_id;
  void *buf;

  // weee - switching
  switch(uc->code) { 

          //Handles trap for excec and checks if the inputs are valid.
      case YALNIX_EXEC:
                TracePrintf(3, "trapKernel: YALNIX_EXEC\n");

                // Check that the pointers are not outisde the pagetable 
                if ( check_pointer_range(uc->regs[0]) 
                  || check_pointer_range(uc->regs[1]) ){

                        TracePrintf(3, "trapKernel (pid %d): error in EXEC, out of range\n", curr_proc->process_id);
                        retval = ERROR;
                        break;
                }
                
                //check that the memory is valid.
                if ( check_pointer_valid(uc->regs[0])
                  || check_pointer_valid(uc->regs[1]) ){

                        TracePrintf(3, "trapKernel (pid %d): error in EXEC, invalid\n", curr_proc->process_id);
                        retval = ERROR;
                        break;
                }

                //check that there are proper permissions
                if ( is_rw(uc->regs[0]) \
                  || is_rw(uc->regs[1]) ){

                        TracePrintf(3, "trapKernel (pid %d): error in EXEC, not rw\n", curr_proc->process_id);
                        retval = ERROR;
                        break;
                }

                char *fname_p = (char *) uc->regs[0]; //filename of prgram to run
                char **arg_p = (char **) uc->regs[1]; //arguements of prog to run
                int file_size = (strlen(fname_p) + 1) * sizeof(char); //size of the filename

                //need to copy the file name into kernel space
                char *filename = (char *) malloc(file_size); 
                ALLOC_CHECK(filename, "exec"); //double check that filename was malloced
                memcpy((void*) filename, (void*) fname_p, file_size); //make a kernel copy of the filename
                TracePrintf(6, "trapKernel: calling exec on %s\n", filename);
                TracePrintf(3, "tk:%s\n", arg_p[0]);

                //begin exec-ing
                kernel_Exec(uc, filename, arg_p); 
                TracePrintf(5, "trapKernel: YALNIX_EXEC End\n");
                break;

      case YALNIX_DELAY:
                TracePrintf(3, "trapKernel: YALNIX_DELAY \n");
                clock_ticks = (int) uc->regs[0];
                retval = kernel_Delay(uc, clock_ticks);
                TracePrintf(5, "trapKernel: YALNIX_DELAY End \n");
                break;

      case YALNIX_FORK:
                TracePrintf(3, "trapKernel: YALNIX_FORK\n");
                retval = kernel_Fork(uc);
                TracePrintf(8, "retval: %d\n", retval);
                TracePrintf(5, "trapKernel: YALNIX_FORK End\n");
                break;

      case YALNIX_EXIT:
                TracePrintf(3, "trapKernel: YALNIX_EXIT\n");
                int status = (int) uc->regs[0];
                kernel_Exit(status, uc);
                TracePrintf(5, "trapKernel: YALNIX_EXIT End\n");
                break;

      case YALNIX_WAIT:
                TracePrintf(3, "trapKernel: YALNIX_WAIT\n");

                 //check if in range
                if ( check_pointer_range(uc->regs[0]) ){
                        TracePrintf(3, "trapKernel (pid %d): error in WAIT, pointer out of range\n", curr_proc->process_id);
                        retval = ERROR;
                        break;
                }
                // check if pages are valid
                if ( check_pointer_valid(uc->regs[0]) ){
                        TracePrintf(3, "trapKernel (pid %d): error in WAIT, page not valid \n", curr_proc->process_id);
                        retval = ERROR;
                        break;
                }
                // check if RW
                if ( is_rw(uc->regs[0]) ){
                        TracePrintf(3, "trapKernel (pid %d): error in WAIT, page not rw\n", curr_proc->process_id);
                        retval = ERROR;
                        break;
                }
        
                // status pointer that will have the return status of the exiting
                // process.
                int *status_pt = (int *) uc->regs[0];
                retval = kernel_Wait(status_pt, uc);
                TracePrintf(5, "trapKernel: YALNIX_WAIT End\n");
                break;

      case YALNIX_GETPID:
                TracePrintf(3, "trapKernel: YALNIX_GETPID\n");
                retval = kernel_GetPid(uc);
                TracePrintf(5, "trapKernel: YALNIX_GETPID End\n");
                break;

      case YALNIX_PIPE_INIT:
                TracePrintf(3, "trapKernel: YALNIX_PIPE_INIT\n");
                // check if in range
                if ( check_pointer_range(uc->regs[0]) ){
                        TracePrintf(3, "trapKernel(pid %d): error in PipeInit, pointer out of range\n", curr_proc->process_id);
                        retval = ERROR;
                        break;
                }
                // check if pages are valid
                if ( check_pointer_valid(uc->regs[0]) ){
                        TracePrintf(3, "trapKernel:(pid %d) error in PipeInit, pointer address not valid \n", curr_proc->process_id);
                        retval = ERROR;
                        break;
                }
                // check if RW
                if ( is_rw(uc->regs[0]) ){
                        TracePrintf(3, "trapKernel(pid %d): error in PipeInit, address not rw\n", curr_proc->process_id);
                        retval = ERROR;
                        break;
                }

                retval = PipeInit((int*) uc->regs[0]);
                TracePrintf(5, "trapKernel: YALNIX_PIPE_INIT End\n");
                break;

      case YALNIX_PIPE_READ:
                TracePrintf(3, "trapKernel: YALNIX_PIPE_READ\n");

                //check if the supposed buffer is valid.
                if( check_string_validity(uc->regs[1], uc->regs[2]) ){
                        retval = ERROR;
                        break;
                }
                memcpy((void*) curr_proc->user_context, (void*) uc, sizeof(UserContext));
                retval = kernel_PipeRead((int) uc->regs[0], (void *) uc->regs[1], (int) uc->regs[2], uc);

                TracePrintf(5, "trapKernel: YALNIX_PIPE_READ End\n");
                break;

      case YALNIX_PIPE_WRITE:
                TracePrintf(3, "trapKernel: YALNIX_PIPE_WRITE\n");

                if( check_string_validity(uc->regs[1], uc->regs[2]) ){
                        retval = ERROR;
                        break;
                }
                retval = kernel_PipeWrite((int) uc->regs[0], (void * ) uc->regs[1], (int) uc->regs[2]);

                TracePrintf(5, "trapKernel: YALNIX_PIPE_WRITE End\n");
                break;

      case YALNIX_BRK:
                TracePrintf(3, "trapKernel: YALNIX_BRK\n");

                // check if in range
                if ( check_pointer_range(uc->regs[0]) ){
                        TracePrintf(0, "trapKernel: pid %d: brk to set is: %d\n", curr_proc->process_id, (int)uc->regs[0]);
                        TracePrintf(3, "trapKernel: error in Brk, pointer out of range\n");
                        retval = ERROR;
                        break;
                }
                // check if pages are valid
                if ( check_pointer_valid(uc->regs[0]) ){
                        TracePrintf(3, "trapKernel (pid %d): error in Brk, pointer address not valid\n", curr_proc->process_id);
                        retval = ERROR;
                        break;
                }
                // check if RW
                if ( is_rw(uc->regs[0]) ){
                        TracePrintf(3, "trapKernel (pid %d): error in Brk, address not rw \n", curr_proc->process_id);
                        retval = ERROR;
                        break;
                }

                void *addr = (void *) uc->regs[0];
                retval = kernel_Brk(addr);

                TracePrintf(5, "trapKernel: YALNIX_BRK End\n");
                break;

       case YALNIX_TTY_WRITE:
                TracePrintf(5, "trapKernel: TTY_WRITE\n");
                if ( check_string_validity(uc->regs[1], uc->regs[2]) ){
                        TracePrintf(3, "trapTTYWRITE: error in string, out of range\n");
                        retval = ERROR;
                        break;
                }

                 tty_id = (int) uc->regs[0];
                 buf = (void *) uc->regs[1];
                 len = (int) uc->regs[2];
                 retval = kernel_TtyWrite(tty_id, buf, len);
                TracePrintf(5, "trapKernel: TTY_WRITE End\n");
                 break;

       case YALNIX_TTY_READ:
                TracePrintf(5, "trapKernel: TTY_READ\n");
                 //check string validity in uc->regs
                if ( check_string_validity(uc->regs[1], uc->regs[2]) ){
                        TracePrintf(3, "trapTTYWRITE: error in string, out of range\n");
                        retval = ERROR;
                        break;
                }

                tty_id = (int) uc->regs[0];
                buf = (void *) uc->regs[1];
                len = (int) uc->regs[2];
                retval = kernel_TtyRead(tty_id, buf, len);
                TracePrintf(5, "trapKernel: TTY_READ End\n");
                break;

        case YALNIX_LOCK_INIT:
                TracePrintf(5, "trapKernel: LOCK_INIT");
                   //check if in range
                if ( check_pointer_range(uc->regs[0]) ){
                        TracePrintf(3, "trapKernel: error in YALNIX_LOCK_INIT, pointer out of range\n");
                        retval = ERROR;
                        break;
                }
                if ( is_rw(uc->regs[0]) ){
                        TracePrintf(3, "trapKernel: error in YALNIX_LOCK_INIT, page not rw\n");
                        retval = ERROR;
                        break;
                }

                retval = kernel_LockInit((int*) uc->regs[0]);
                TracePrintf(5, "trapKernel: LOCK_INIT End");
                break;

        case YALNIX_LOCK_ACQUIRE:
                TracePrintf(5, "trapKernel: LOCK_ACQUIRE\n");
                retval = kernel_Acquire((int) uc->regs[0]);
                TracePrintf(5, "trapKernel: LOCK_ACQUIRE\n");
                break;

        case YALNIX_LOCK_RELEASE:
                TracePrintf(5, "trapKernel: LOCK_RELEASE\n");
                retval = kernel_Release((int) uc->regs[0]);
                TracePrintf(5, "trapKernel: LOCK_RELEASE End\n");
                break;

        case YALNIX_CVAR_INIT:
                TracePrintf(5, "trapKernel: CVAR_INIT\n");
                retval = kernel_CvarInit((int*)uc->regs[0]);
                TracePrintf(5, "trapKernel: CVAR_INIT End\n");
                break;

        case YALNIX_CVAR_SIGNAL:
                TracePrintf(5, "trapKernel: CVAR_SIGNAL\n");
                retval = kernel_CvarSignal((int)uc->regs[0]);
                TracePrintf(5, "trapKernel: CVAR_SIGNAL End\n");
                break;

        case YALNIX_CVAR_BROADCAST:
                TracePrintf(5, "trapKernel: CVAR_BROADCAST\n");
                retval = kernel_CvarBroadcast((int)uc->regs[0]);
                TracePrintf(5, "trapKernel: CVAR_BROADCAST End\n");
                break;
          
        case YALNIX_CVAR_WAIT:
                TracePrintf(5, "trapKernel: CVAR_WAIT\n");
                retval = kernel_CvarWait((int)uc->regs[0], (int)uc->regs[1]);
                TracePrintf(5, "trapKernel: CVAR_WAIT End\n");
                break;

        case YALNIX_RECLAIM:
                TracePrintf(5, "trapKernel: RECLAIM\n");
                retval = kernel_Reclaim((int)uc->regs[0]);
                TracePrintf(5, "trapKernel: RECLAIM End\n");
                break;   

        //Trouble, this should never happen
        default:
                TracePrintf(3, "Unrecognized syscall: %d\n", uc->code);
                break;
    }



        // set return value  
        uc->regs[0] = retval;
}

// Handles trapping the clock and moving onto the next process
void trapClock(UserContext *uc)
{
        TracePrintf(1, "trapClock ### start \n");
        memcpy((void*) curr_proc->user_context, (void*) uc, sizeof(UserContext));

        Node *temp = all_procs->head;
        pcb *p;
        
        //Print some info about the state of the processes
        //Super valuable for debugging
        TracePrintf(5, "ALL PROCS\n");
        while (temp!=NULL){
                p = (pcb*) temp->data;
                TracePrintf(3, "%d\n", p->process_id);
                temp = temp->next;
        }

        TracePrintf(5, "READY PROCS\n");
        temp = ready_procs->head;
        while (temp!=NULL){
                p = (pcb*) temp->data;
                TracePrintf(3, "%d\n", p->process_id);
                temp = temp->next;
        }

        TracePrintf(5, "ZOMBIE PROCS\n");
        temp = zombie_procs->head;
        while (temp!=NULL){
                p = (pcb*) temp->data;
                TracePrintf(3, "%d\n", p->process_id);
                temp = temp->next;
        }


        //Checks if a process done being delayed
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
        } else {
                TracePrintf(3, "trapClock: no process to switch to gonna keep going");
        }



        TracePrintf(1, "trapClock ### end \n");
}

// Handles illegal actions not pertaining to memory.
// Attempts to exit the process. 
void trapIllegal(UserContext *uc)
{
        TracePrintf(1, "trapIllegal ### start: now exiting \n");
        TracePrintf(6, "\t Illegal pid = %d \n \t status: %d",
            curr_proc->process_id, uc->code);
      	int status = -1;
      	kernel_Exit(status, uc);
        uc->regs[0] = status;
}

//Handles illegal memory access.
//If the heap or stack need more space, we give it to them,
//so long as they do not grow into eachother
void trapMemory(UserContext *uc)
{
        pcb* curr = curr_proc;
        TracePrintf(0, "Illegal Memory code %d in pid %d\n", uc->code, curr->process_id);
        TracePrintf(6, "\t address %p \n \t sp %p \n \t pc %p \n \t \t brk %p \n", uc->addr, uc->sp, uc->pc, curr->brk_address);
        
        //memcpy((void*) curr->user_context, (void*) uc, sizeof(UserContext));

        int status = -1;

        if (uc->code == YALNIX_ACCERR) {
          TracePrintf(6, "\t Process had access error at %p \n", uc->code);
          kernel_Exit(status, uc);
        }
        else if (uc->code == YALNIX_MAPERR) {
          TracePrintf(6, "\t Process had a mapping error\n");
          if ((unsigned int)uc->addr < curr->brk_address || 
                (unsigned int)uc->addr > (unsigned int) uc->sp){
                  TracePrintf(6, "\t exiting, real mapping err so sorry \n");
                  int brk_over = ((unsigned int)uc->addr < curr->brk_address);
                  TracePrintf(6, "\t error: uc->addr %u,  brk_address %d \n", (unsigned int)uc->addr, curr->brk_address);
                  kernel_Exit(status, uc);
                  // uc->regs[0] = status;
          }
        }

      unsigned int addr_page = DOWN_TO_PAGE(uc->addr) >> PAGESHIFT;
      unsigned int stack_top_page = VMEM_1_LIMIT >> PAGESHIFT;
      unsigned int usr_brk_page = UP_TO_PAGE(curr->brk_address) >> PAGESHIFT;
      unsigned int usr_heap_bottom = curr->heap_start_pg;
      
      int i; 

      // check if the requested adreess would mess up the heap,
      // i.e there should be at least one page to make it work
      // two not to mess it up
      TracePrintf(6, "trapKernelMemory: \t addr_page %u, brk page %u", addr_page, usr_brk_page);
      if ( (addr_page - usr_brk_page) <= 2 && ((int)addr_page - (int)usr_brk_page) < 0) {
            TracePrintf(6, "Illegal Memory: process out of memory \n");
            kernel_Exit(status, uc);
      }   

      unsigned int vm_pg_start = addr_page - 128;
      unsigned int vm_pg_end = stack_top_page - 128;
      struct pte *temp;
      for (i = vm_pg_start; i<= vm_pg_end; i++){
                temp = (curr->region1_pt + i);

                // no page allocated for region 1 pt
                if (temp->valid == (u_long) 0x0) {
                        if (list_count(empty_frame_list) < 1) {
                                TracePrintf(6, "Illegal Memory: not enough frames to grow stack \n");
                                kernel_Exit(status, uc);
                        }

                        // set page to valid
                        temp->valid = (u_long) 0x1;
                        temp->prot = (u_long) (PROT_READ | PROT_WRITE);

                        // map  aframe to it
                        int pfn = (int) list_pop(empty_frame_list);
                        temp->pfn = (u_long) ( (pfn * PAGESIZE) >> PAGESHIFT);

                }
      }
      // flush me baby flush me baby
      WriteRegister(REG_TLB_FLUSH, TLB_FLUSH_ALL);
      TracePrintf(0, "Illegal Memory: Surived\n");

        
}

// Handles Illegal Math from the NSA.
void trapMath(UserContext *uc)
{
        TracePrintf(0, "trapMath ### start: now exiting; pid %d\n", curr_proc->process_id);
      	int status = -1;
      	kernel_Exit(status, uc);
        uc->regs[0] = status;
}

//Trap when we are done reciving a buffer from the terminal. 
//We then write it to a pipe.
void trapTTYReceive(UserContext *uc)
{

        TracePrintf(0, "trapTTYReceive ### start\n");

        // find the tty by uc->code
        int tty_id = uc->code;
        TTY *tty = NULL;
        Node * node =  ttys->head;
        while (node != NULL){
                if ( ((TTY*)(node->data))->tty_id == tty_id){
                        tty = (TTY *) node->data;
                        break;
                }
                node = node->next;
        }
 

        if (tty == NULL) {
                TracePrintf(6, "TtyWrite: ERROR; tty %d out of bounds \
                                - should not happen \n", tty_id);
                return;// ERROR;
        }

        //allocate new buffer
        Buffer *new = (Buffer *) malloc(sizeof(Buffer));
        new->buf = (char *) calloc(TERMINAL_MAX_LINE, sizeof(char));
        // call TtyRecieve
        new->len = TtyReceive(tty->tty_id, new->buf, TERMINAL_MAX_LINE);
        TracePrintf(6, "recieved buffer %s, of len %d from teriminal %d", (char *) new->buf, new->len, tty->tty_id);



        // add the created buffer on the list of buffers
        // for that tty
        list_add(tty->buffers, new);

        

        // if there is anyone on the to_read list, 
        // put it back on the ready list
        int len = new->len;
        while ( (list_count(tty->to_read) > 0) && (len > 0) ) {
            TracePrintf(0, "trapTTYReceive to the next and beyond\n");
                pcb *waiter = list_pop(tty->to_read);
                list_add(ready_procs, waiter);
                len = len - waiter->read_length;
        }

        TracePrintf(0, "trapTTYReceive ### end me baby\n");

}

//Trap when we are done writing to a terminal
//we then can either keep writing until the buffer is empty, 
//or if the buffer is already empty, goto the next process that
//needs something written.
void trapTTYTransmit(UserContext *uc)
{
        TracePrintf(0, "trapTTYTransmit ### start \n");

        // find the tty by uc->code
        int tty_id = uc->code;
        TTY *tty = NULL;
        Node *node =  ttys->head;

        //find the tty that is requested by the uc
        while (node != NULL){
                if ( ((TTY*)(node->data))->tty_id == tty_id){
                        tty = (TTY *) node->data;
                        break;
                }
                node = node->next;
        }

        if (tty == NULL) {
                TracePrintf(6, "TtyWrite: ERROR; tty %d out of bounds \
                                - should not happen \n", tty_id);
                return;
        }

        TracePrintf(0, "COUNT: %d\n", tty->to_write->count);

        //Figure out what to write
        if( list_count(tty->to_write) <= 0)
        {
                TracePrintf(3, "TtyWrite: Nothing to write on %d\n", tty_id);
        } else {
                TracePrintf(6, "TtyWrite: \n about to pop a waiter\n");
                if(tty->to_write->head == NULL)
                  TracePrintf(0, "hi %d\n\n\n", tty->to_write->count);
                if(tty->to_write->head->data == NULL)
                  TracePrintf(0, "failure\n\n\n");
                pcb *writer = (pcb *)list_pop(tty->to_write);

               TracePrintf(6, "TtyWrite: \n done popping a waiter\n");
             
                // check if we need to write more then once,
                // otherwise call TtyTransmit
                int remains = writer->buffer->len - TERMINAL_MAX_LINE;

                if (remains > 0 ) {
                        writer->buffer->len = remains;
                        writer->buffer->buf = writer->buffer + TERMINAL_MAX_LINE;

                        list_add(tty->to_write, (void *) writer);

                        if (remains > TERMINAL_MAX_LINE) {
                                TtyTransmit(tty_id, writer->buffer->buf, TERMINAL_MAX_LINE);
                        }
                        else {
                                TtyTransmit(tty_id, writer->buffer->buf, writer->buffer->len);
                        }
                }
                else {
                        // check if there are waiting process, and call
                        // the transmit for them (see where I break it up
                        // in the syscall)
                        TracePrintf(6, "TtyWrite: \n nothing remains, adding to ready\n");

                        list_add(ready_procs, writer);
                        pcb *next;
                        if (list_count(tty->to_write) > 0) {
                                next = list_pop(tty->to_write);
                                list_add(tty->to_write, (void *) next);
                                if(tty->to_write->head == NULL)
                                  TracePrintf(0, "cukdasfdsa\n");

                                int next_len = (int) next->buffer->len;
                                if (next_len > TERMINAL_MAX_LINE){
                                        TtyTransmit(tty_id, next->buffer->buf, TERMINAL_MAX_LINE);
                                }
                                else {
                                        TtyTransmit(tty_id, next->buffer->buf, next_len);
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

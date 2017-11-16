/*=============================================================================
 |   Assignment:   Yalnix Operating System
 |
 |       Author:  P.J.Flathers and B.Korbar
 |     Language:  C
 |   To Compile:  part of the yalnix os --- see makefile in the root folder
 |
 |        Class:  COSC 58 - Operating Systems
 |   Instructor:  Dr. Sean Smith
 |     Due Date:  Nov 16th, 2017
 |
 +-----------------------------------------------------------------------------
 |
 |  Purpose:  Basic implementation of reading and writing to the terminal
 |
 *===========================================================================*/

/* 
 * Local includes
 */
#include "globals.h"
#include "list.h"
#include "tty.h"
#include "kernel.h"


/*------------------------------------------------- kernel_TtyRead -----
|  Function kernel_TtyRead
|
|  Purpose:  "reads" len chars from a tty buffer of the corresponding TTY, 
|           by copying it's context (properly formatted) to the output 
|           buffer 
|
|  Parameters:
|      int tty_id (IN) - id of the tty we read from
|      void *buf (OUT) - pointer to the char array of the buffer we are
|                       reading into
|      int len (IN)    - length of the reading buffer
|
|  Returns:  length of the successful read, or ERROR
*-------------------------------------------------------------------*/
int kernel_TtyRead(int tty_id, void *buf, int len)
{
        TracePrintf(3, "TtyRead ### Start\n");

        // look up tty by it's id
        TTY *tty = NULL;
        Node * node =  ttys->head;

        while (node->next != NULL){
                if ( ((TTY*)node->data)->tty_id == tty_id){
                        tty = (TTY *) node->data;
                        break;
                }
                node = node->next;
        }
        //edge case
        if ( ((TTY*)node->next->data)->tty_id == tty_id) {
                tty = (TTY*)node->next->data;
        }
        if (tty == NULL) {
                TracePrintf(3, "TtyWrite ERROR; tty %d out of bounds \
                                - should not happen \n", tty_id);
                return ERROR;
        }

        Buffer *popped_buf = list_pop(tty->buffers);
        // if there are no stored bufferst to read from switch and 
        // wait untill inetrrupt gets us
        if (!popped_buf) {
                TracePrintf(6, "ttwRead: \t no buffer, wait for interrupt\n");
                curr_proc->read_length = len;
                list_add(tty->to_read, (void *)curr_proc);
                TracePrintf(6, "ttwRead: \t curr proc on the to_read list\n");
                goto_next_process(curr_proc->user_context, 0);


                // should wake up here after enough process switches
                TracePrintf(6, "ttyRead: \t woken after no buffer\n");
                popped_buf = list_pop(tty->buffers);

        }

        // check if our local buffer is full 
        if (!popped_buf) {
            TracePrintf(3, "ttyRead ERROR: ayayayayay\n");
            return ERROR;    
        }

        // memcpy that buffer to the in/out one
        TracePrintf(6, "ttyRead: \t about to read buffer\n");
        memcpy(buf, popped_buf->buf, len);
        TracePrintf(6, "ttyRead: \t read buffer, reset pcb read counter\n");
        curr_proc->read_length = 0;

        /* here is the thing that I'm unsure about, 
        we should somehow store the rest of the buffer if we
        haven't read the entire thing */
        int full_len = strlen(popped_buf->buf);
        int to_store = full_len - len; 

        // if there is anything left
        if (to_store > 0) {
                TracePrintf(6, "ttyRead: \t didn't read entire thing\n");
                len = len - to_store;
                // buffer we want to store will be the pooped buffer
                // shifted by the length we have read
                char *stash_buf = popped_buf->buf + len;
                memcpy(popped_buf->buf, stash_buf, to_store);
                popped_buf->len = to_store;
                list_add(tty->buffers, popped_buf);
        }
        else {
                TracePrintf(6, "ttyRead: \t read entire thing first time\n");
        }


        TracePrintf(3, "TtyRead ### end\n");
	return len;
}

/*------------------------------------------------- kernel_TtyWrite -----
|  Function kernel_TtyWrite
|
|  Purpose:  "writes" len chars from a input buffer by copying it's context (properly 
|           formatted) to the output buffer of the corresponding TTY
|           
|
|  Parameters:
|      int tty_id (IN) - id of the tty we write to
|      void *buf (IN) - pointer to the char array of the buffer we are
|                       writing from
|      int len (IN)    - length of the reading buffer
|
|  Returns:  length of the successful write, or ERROR
*-------------------------------------------------------------------*/
int kernel_TtyWrite(int tty_id, void *buf, int len)
{
        TracePrintf(3, "TtyWrite ### Start\n");

	   // look up tty by it's id
        TTY *tty = NULL;
        Node * node =  ttys->head;
        while (node->next != NULL){
                if ( ((TTY*)node->data)->tty_id == tty_id){
                        tty = (TTY *) node->data;
                        break;
                }
                node = node->next;
        }
        

        if (tty == NULL) {
                TracePrintf(3, "TtyWrite: ERROR; tty %d out of bounds \
                                - should not happen \n", tty_id);
                return ERROR;
        }

        TracePrintf(6, "ttyWrite: \t not dead yet \n");
        /* allocate new write buffer in the current process */ 

        // there shouldn't be anything left, but might be
        // if process went to exit before freeing buffers
        if (curr_proc->buffer->buf == NULL){
                free(curr_proc->buffer->buf);
        }

        TracePrintf(6, "TtyWrite:\t Alloc (or realloc) pcb buffer to the heap \n");
        // calloc so that I don't have to bzero
        curr_proc->buffer->buf = (char *) calloc(len, sizeof(char));
        ALLOC_CHECK(curr_proc->buffer->buf, "ttyWrite - process buffer on the heap");
        // copy buffer in it and set the length ot write buffer
        //memcpy(((Buffer *)curr_proc->buffer)->buf, buf, len);
        strncpy(((Buffer *)curr_proc->buffer)->buf, buf, len);
        curr_proc->buffer->len = len;
        TracePrintf(6, "TtyWrite:\t Done with alloc (or realloc) pcb buffer to the heap \n");


        // add current proc to to_write of the current tty
        list_add(tty->to_write, (void *) curr_proc); 
        TracePrintf(6, "TtyWrite: \t curr proc (%d) added to_write list \n", curr_proc->process_id);
      
        // if we are the first/only, we can write now, 
        // otherwise wait for kernel
        if (list_count(tty->to_write) == 1) {
                TracePrintf(6, "TtyWrite: \t curr proc the only writer; (%d) send to terminal", curr_proc->process_id);
                if (len > TERMINAL_MAX_LINE) {
                                TtyTransmit(tty_id, buf, TERMINAL_MAX_LINE);
                }
                else {
                        TracePrintf(6, "TtyWrite: \t printing len %d", len);
                        TtyTransmit(tty_id, buf, len);
                }
                //return len;
        }
        else{
                TracePrintf(6, "TtyWrite: \t there are writers in front of me, wait \n");
        }


        // move on
       // list_add(blocked_procs, curr_proc);
        goto_next_process(curr_proc->user_context, 0);

        // here, the waiting process should return after tty trap
        TracePrintf(6, "TtyWrite: \t proc %d returned after waiting \n", curr_proc->process_id);
        TracePrintf(3, "TtyWrite ### End, returning %d\n", len);
        return len;
}

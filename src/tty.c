#include "global.h"
#include "list.h"
#include "tty.h"
#include "kernel.h"


int TtyRead(int tty_id, void *buf, int len)
{
        // if there are no stored bufferst to read from
        // switch


        // check if our local buffer is full 

        // memcpy that buffer to the in/out one

        // update bookkeeping

        // if we didn't read the entire thing, put 
        // it back on the tty->buffer's list



	return len;
}

int TtyWrite(int tty_id, void *buf, int len)
{
        TracePrintf(3, "TtyWrite ### Start\n");

	// look up tty by it's id
        TTY *tty = NULL;
        Node * node =  ttys->head;
        while (node->next != NULL){
                if (node->data->id == tty_id){
                        tty = (TTY *) node->data;
                        break;
                }
                node = node->next;
        }
        //edge case
        if (node->next->data->id == tty_id) {
                tty = tty->next->data;
        }

        if (tty == NULL) {
                TracePrintf(6, "TtyWrite: ERROR; tty %d out of bounds \
                                - should not happen \n", tty_id);
                return ERROR;
        }


        /* allocate new write buffer in the current process */ 

        // there shouldn't be anything left, but might be
        // if process went to exit before freeing buffers
        if (curr_proc->buffer->buf != NULL){
                free(curr_proc->buffer->buf);
        }

        TracePrintf(6, "TtyWrite:\t Alloc (or realloc) pcb buffer to the heap \n");
        curr_proc->buffer->buf = (char *) malloc(len * sizeof(char));
        ALLOC_CHECK(curr_proc->buffer->buf, "ttyWrite - process buffer on the heap");
        // copy buffer in it and set the length ot write buffer
        memcpy(((Buffer *)curr_proc->buffer)->buf, buf, len);
        curr_proc->buffer->len = len;
        TracePrintf(6, "TtyWrite:\t Done with alloc (or realloc) pcb buffer to the heap \n");


        // add current proc to to_write of the current tty
        list_add(tty->to_write, (void *) curr_proc); 
        TracePrintf(6, "TtyWrite: \t curr proc (%d) added to_write list \n", curr_proc->process_id);
      
        // if we are the first/only, we can write now, 
        // otherwise wait for kernel
        if (list_count(tty->to_write) == 1) {
                TracePrintf(6, "TtyWrite: \t curr proc (%d) send to terminal", curr_proc->process_id);
                if (len > TERMINAL_MAX_LINE) {
                                TtyTransmit(tty_id, buf, TERMINAL_MAX_LINE);
                }
                else {
                        TtyTransmit(tty_id, buf, len);
                }
        }
        else{
                TracePrintf(6, "TtyWrite: \t there are writers in front of me, wait \n");
        }


        // move on
        goto_next_process(curr_proc->user_context, 0);

        // here, the waiting process should return after tty trap
        TracePrintf(6, "TtyWrite: \t proc %d returned after waiting \n" curr_proc->process_id);
        TracePrintf(3, "TtyWrite ### End, returning %d\n", len);
        return len;
}

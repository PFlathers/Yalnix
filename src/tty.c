#include "globals.h"
#include "list.h"
#include "tty.h"
#include "kernel.h"


int TtyRead(int tty_id, void *buf, int len)
{
        TracePrintf(3, "TtyRead ### Start\n");

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

        Buffer *popped_buf = list_pop(tty->buffers);
        // if there are no stored bufferst to read from switch and 
        // wait untill inetrrupt gets us
        if (!popped_buf) {
                TracePrintf(6, "ttwRead: \t no buffer, wait for interrupt\n");
                curr_proc->read_len = len;
                list_add(tty->to_write, curr_proc);
                goto_next_process(curr_proc->user_context, 0);


                // should wake up here after enough process switches
                TracePrintf(6, "ttwRead: \t woken after no buffer\n");
                popped_buf = list_pop(tty->buffers);

        }

        // check if our local buffer is full 
        if (!popped_buf) {
            TracePrintf(6, "ttwRead: ayayayayay\n");
            return ERROR;    
        }

        // memcpy that buffer to the in/out one
        TracePrintf(6, "ttwRead: \t about to read buffer\n");
        memcpy(buf, popped_buf->buf, len);
        TracePrintf(6, "ttwRead: \t read buffer, reset pcb read counter\n");
        curr_proc->read_len = 0;

        /* here is the thing that I'm unsure about, 
        we should somehow store the rest of the buffer if we
        haven't read the entire thing */
        int full_len = strlen(popped_buf->buf);
        int to_store = full_len - len; 

        // if there is anything left
        if (to_store > 0) {
                TracePrintf(6, "ttwRead: didn't read entire thing\n");
                len = len - to_store;
                // buffer we want to store will be the pooped buffer
                // shifted by the length we have read
                char *stash_buf = popped_buf->buf + len;
                memcpy(popped_buf->buf, stash_buf, to_store);
                popped_buf->len = to_store;
                list_add(tty->buffers, popped_buf);
        }
        else {
                TracePrintf(6, "ttwRead: read entire thing first time\n");
        }


        TracePrintf(3, "TtyRead ### end\n");
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

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

        


        // allocate new write buffer in the current process
        // copy buffer in it
        // set the length ot write buffer


        // add current proc to to_writers of the current tty

        // if we are the first/only, we can transmin now, 
        // otherwise wait for kernel

        // move on
        goto_next_process(curr_proc->user_context, 0);

        TracePrintf(3, "TtyWrite ### End, returning %d\n", len);
        return len;
}

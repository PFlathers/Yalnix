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
	// look up tty by it's id

        // allocate new write buffer in the current process
        // copy buffer in it
        // set the length ot write buffer


        // add current proc to to_writers of the current tty

        // if we are the first/only, we can transmin now, 
        // otherwise wait for kernel

        // move on
        goto_next_process(curr_proc->user_context, 0);


        return len;
}

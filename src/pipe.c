#include <hardware.h>
#include "globals.h"
#include "pipe.h"
#include "kernel.h"
#include <string.h>

/*
 * Creates a new pipe. pipe_id will return the id of the pipe to userland
 */
int PipeInit(int *pipe_idp)
{
        TracePrintf(3, "PipeInit ### start\n");

	// allocate pipe, buffer, and queue for the pipe
        Pipe *pipe = (Pipe *) malloc(sizeof(Pipe));
        ALLOC_CHECK(pipe, "PipeInit");

        pipe->buffer = (char *) malloc(sizeof(Pipe));
        ALLOC_CHECK(pipe->buffer, "PipeInit-Buffer");

        pipe->pipe_queue = (List *) init_list();
        ALLOC_CHECK(pipe->pipe_queue, "PipeInit-Queue");
        

        // set pipe id, length
        pipe->id = available_lock_id++;
        pipe->length = 0;
        pipe->exp_length = 0;
        pipe->pipe_queue->head = NULL;

        // bookkeeping
        list_add(pipes, (void*) pipe);

        // save id in the user id pipe_idp
        *pipe_idp = pipe->id;

        TracePrintf(3, "PipeInit ### end\n");
        return SUCCESS;
}

/*
 * Read and return x amount of characters from the pipe. 
 * Arguement:
 *      pipe_id: the pipe we will be reading from
 *      buf: the buf we will be writing to
 *      len: the number of characters to read
 */
int kernel_PipeRead(int pipe_id, void *buf, int len, UserContext *uc)
{
        TracePrintf(3, "PipeRead ### start\n");

        Node *node = pipes->head;
        Pipe *pipe = NULL;
        int gp_flag = 0;
        // find the pipe in the global list
        while(node != NULL){
                if ( ((Pipe*)(node->data))->id == pipe_id){
                        pipe = (Pipe *)node->data;
                        break;
                }
                node = node->next;
        }

        // if there are no requested pipes,return error
        if (!pipe){
                return ERROR;
        }
        // check for maximum length
        if (len > MAX_PIPE_LEN){
                return ERROR;
        }

        // if there are too few chars in the pipe, add it to the
        // waiting list and switch to new process
        if (len > pipe->length){
                pipe->exp_length = len;
                curr_proc->pipe_lenght = len;
                list_add(pipes, (void *) pipe);
                gp_flag = 1;
        }
        
        //reading x chars into the return buffer
        strncpy((char*) buf, pipe->buffer, len);

        //move stuff down the pipe 
        strcpy(pipe->buffer, &(pipe->buffer[len]));
       
        
        // zero out the old shifted stuff
        bzero(pipe->buffer + (MAX_PIPE_LEN - len), len);

        // Book keeping
        pipe->length -= len;

        TracePrintf(3, "PipeRead ### end\n");
        if(gp_flag)
                goto_next_process(curr_proc->user_context, 1);

	return len;
}

/*
 * Writes the given buffer to the pipe
 * Arguements:
 *      pipe_id: the pipe we will be writing to
 *      buf: the buffer that contains what we will be writing
 *      len: the length of the buffer
 */
int kernel_PipeWrite(int pipe_id, void *buf, int len)
{
        TracePrintf(3, "PipeWrite ### start\n");
       Node *node = pipes->head;
        Pipe *pipe = NULL;

        // find the pipe in the global list
        while(node != NULL){
                if ( ((Pipe*)node->data)->id == pipe_id){
                        pipe = (Pipe *)node->data;
                        break;
                }
                node = node->next;
        }

        // if there are no requested pipes,return error
        if (!pipe){
                return ERROR;
        }

        // check if we can write the full length
        if (len > (MAX_PIPE_LEN - pipe->length)){
                return ERROR;
        }

        // actually writing from buffer to the pipe's buffer
        // using mem move to avoid mess with memcopy, 
        // patrick might find better way of doing it
        memmove((void *)(pipe->buffer + pipe->length), (void *)buf, len);
        pipe->length += len;

        if(pipe->pipe_queue->count == 0 ){
                TracePrintf(0, "PipeWrite ### pipe_queue is empty\n");
        }
        // handle processes in the queue
        else if(pipe->pipe_queue->count != 0){
                TracePrintf(3, "PipeWrite ### pipequeue is not empty. Handling it\n");
                pcb *queued_proc = (pcb *) list_pop(pipe->pipe_queue);
                if (queued_proc){
                        int required_length = queued_proc->pipe_lenght;
                        // if not enough chars, return to the waiting queue
                        if (required_length > pipe->length){
                                list_add(pipe->pipe_queue, (void *) queued_proc);
                        }
                        else{
                                list_add(ready_procs, (void *) queued_proc);
                        }
                }
        }
        TracePrintf(3, "PipeWrite ### end\n");
	return len;
}

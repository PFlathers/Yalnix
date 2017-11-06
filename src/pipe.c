#include "globals.h"
#include "pipe.h"
#include "kernel.h"


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
        pipe->id = glob_resource_list++;
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

int kernel_PipeRead(int pipe_id, void *buf, int len)
{
        TracePrintf(3, "PipeRead ### start\n");

        Node *node = pipes->head;
        Pipe *pipe = NULL;
        // find the pipe in the global list
        while(node/*->next*/ != NULL){
                if ( ((Pipe*)(node->data))->id == pipe_id){
                        pipe = (Pipe *)node->data;
                        break;
                }
                node = node->next;
        }

        // edge case 
        /*
        if ( ((Pipe*)node->next->data)->id == pipe_id){
                pipe = ((Pipe *) node->next->data);//->id;
        }
*/

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
                goto_next_process(curr_proc->user_context, 0);
        }


        /* Reading starts, god help me */

        // memcpy((void *)buf, (void *)pipe->buffer, len);
        // EDIT: run into bugs with memcpy
        // trying memmove based on https://stackoverflow.com/a/4415926
        // (segfaulted, dunno why yet)

        memmove((void *)buf, (void *)pipe->buffer, len);
        // if there is anything else in the pipe, move it forward
        //memcpy((void *)pipe->buffer, (void *)(pipe->buffer + len), MAX_PIPE_LEN - len);
        memmove((void *)pipe->buffer, (void *)(pipe->buffer + len), MAX_PIPE_LEN - len);
        // zero out the old shifted stuff
        bzero(pipe->buffer + (MAX_PIPE_LEN - len), len);

        // Book keeping
        pipe->length -= len;

        TracePrintf(3, "PipeRead ### end\n");
	return(len);
}

int kernel_PipeWrite(int pipe_id, void *buf, int len)
{
        TracePrintf(3, "PipeWrite ### start\n");
       Node *node = pipes->head;
        Pipe *pipe = NULL;

        // find the pipe in the global list
        while(node/*->next*/ != NULL){
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

// replace by kernel_Reclaim
// int PipeDestroy(int pipe_id)
// {
// 	return 0;
// }

#include "globals.h"
#include "pipe.h"


int PipeInit(int *pipe_idp)
{
        TracePrintf(3, "PipeInit ### start\n");

	// allocate pipe, buffer, and queue for the pipe
        Pipe *pipe = (Pipe *) malloc(sizeof(Pipe));
        ALLOC_CHECK(pipe, "PipeInit");

        pipe->buffer = (char *) malloc(sizeof(Pipe));
        ALLOC_CHECK(pipe->buffer, "PipeInit-Buffer");

        pipe->queue = (List *) init_list();
        ALLOC_CHECK(pipe->queue, "PipeInit-Queue");
        

        // set pipe id, length
        pipe->id = glob_id_list++;
        pipe->length = 0;
        pipe->exp_length = 0;
        pipe->queue->head = NULL;

        // bookkeeping
        list_add(pipes, (void*) pipe);

        // save id in the user id pipe_idp
        *pipe_idp = pipe->id;

        TracePrintf(3, "PipeInit ### end\n");
        return SUCCESS;
}

int PipeRead(int pipe_id, void *buf, int len)
{
        TracePrintf(3, "PipeRead ### start\n");

        Node *node = pipes->head;
        Pipe *pipe;

        // find the pipe in the global list
        while(node->next != NULL){
                if (node->data->id == pipe_id){
                        pipe = (pipe *)node->data;
                        break;
                }
                node = node->next;
        }

        // edge case 
        if (node->next->data->id == pipe_id){
                pipe = (pipe *) node->next->data->id;
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
                list_add(pipes, (void *) pipe);
                goto_next_process(curr_proc->user_context, 0);
        }


        /* Reading starts, god help me */

        // memcpy((void *)buf, (void *)pipe->buffer, len);
        // EDIT: run into bugs with memcpy
        // trying memmove based on https://stackoverflow.com/a/4415926
        // (segfaulted, dunno why yet)

        memmove((void *)buf, (void *)pipe->buf, len);
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

int PipeWrite(int pipe_id, void *buf, int len)
{
	return 0;
}

int PipeDestroy(int pipe_id)
{
	return 0;
}

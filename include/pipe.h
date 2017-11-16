#ifndef _PIPE_H_
#define _PIPE_H_

#include <hardware.h>
#define MAX_PIPE_LEN 1024

typedef struct _PIPE {
  int id;
  int length;
  int exp_length;
  char *buffer;

  List *pipe_queue;
} _PIPE;
typedef struct _PIPE Pipe;

//creates a new pipe
int PipeInit(int *pipe_idp);

// reads a certain number of characters from the pipe
int kernel_PipeRead(int pipe_id, void *buf, int len, UserContext *uc);

// writes a certain number of chacters to the pipe
int kernel_PipeWrite(int pipe_id, void *buf, int len);

#endif

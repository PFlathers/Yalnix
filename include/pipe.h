#ifndef _PIPE_H_
#define _PIPE_H_

#define MAX_PIPE_LEN 1024

typedef struct _PIPE {
  int id;
  int length;
  int exp_length;
  char *buffer;

  List *pipe_queue;
} _PIPE;
typedef struct _PIPE Pipe;

int PipeInit(int *pipe_idp);

int PipeRead(int pipe_id, void *buf, int len);

int PipeWrite(int pipe_id, void *buf, int len);

//int PipeDestroy(int pipe_id);

#endif

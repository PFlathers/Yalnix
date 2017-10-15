#ifndef _PIPE_H_
#define _PIPE_H_

typedef struct _PIPE {
  int id;
  int length;
  char *buffer;


  // probably something else, we should
  // erad the textbook
  
} _PIPE;
typedef struct _PIPE Pipe;

int PipeInit(int *pipe_idp);

int PipeRead(int pipe_id, void *buf, int len);

#endif

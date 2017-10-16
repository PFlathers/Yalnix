#ifndef _TTY_H_
#define _TTY_H_

#include "list.h"


/* Something to hold each buffer in */
typedef struct _Buffer { 
  void *buf;  // starting logical addr
  int len; // bytelength
} _BUFFER;

typedef struct _Buffer Buffer;


/* Terminal prototype */
typedef struct _TTY {
	int tty_id; // 1-4, given in assignment

	List *to_write;
	List *buffers;

	//missing probz a lot
} _TTY;

typedef struct _TTY TTY;

int TtyRead(int tty_id, void *buf, int len);

int TtyWrite(int tty_id, void *buf, int len);

#endif

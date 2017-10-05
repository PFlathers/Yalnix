#ifndef _TTY_H_
#define _TTY_H_

#include "list.h"


/* Something to hold each buffer in */
typedef struct _Buffer { 
  void *buf;  // starting logical addr
  int len; // bytelength
} _BUFFER;

typedef struct _Buffer buffer;


/* Terminal prototype */
typedef struct _TTY {
	int tty_id;

	node *to_write;
	node *buffers;

	//missing probz a lot
} _TTY;

typedef struct _TTY tty;


#endif
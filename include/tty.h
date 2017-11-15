/*=============================================================================
 |   Assignment:   Yalnix Operating System
 |
 |       Author:  P.J.Flathers and B.Korbar
 |     Language:  C
 |   To Compile:  part of the yalnix os --- see makefile in the root folder
 |
 |        Class:  COSC 58 - Operating Systems
 |   Instructor:  Dr. Sean Smith
 |     Due Date:  Nov 16th, 2017
 |
 +-----------------------------------------------------------------------------
 |
 |  Purpose:  header for the TTY implementation; source can be found in 
 |              ../src/tty.c; testing can be found in ../userland/tty.c
 |
 *===========================================================================*/

#ifndef _TTY_H_
#define _TTY_H_


/* 
 * System includes 
 */
#include "list.h"


/* 
Datastructure to hold each buffer in - mostly for
convenience 
*/

typedef struct _Buffer { 
  void *buf;  // starting logical addr
  int len; // len of the buffer
} _BUFFER;

typedef struct _Buffer Buffer;


/* Terminal data structure prototype */
typedef struct _TTY {
	int tty_id; // 1-4, given in assignment

	List *to_write; // list of pcbs waiting to write
        List *to_read;   // list of pcbs waiting to read
	List *buffers;  // buffers associated to that 
                        // particular tty

} _TTY;

typedef struct _TTY TTY;

/* Public facing function calls */ 

/*------------------------------------------------- kernel_TtyRead -----
|  Function kernel_TtyRead
|
|  Purpose:  "reads" len chars from a tty buffer of the corresponding TTY, 
|           by copying it's context (properly formatted) to the output 
|           buffer 
|
|  Parameters:
|      int tty_id (IN) - id of the tty we read from
|      void *buf (OUT) - pointer to the char array of the buffer we are
|                       reading into
|      int len (IN)    - length of the reading buffer
|
|  Returns:  length of the successful read, or ERROR
*-------------------------------------------------------------------*/
int kernel_TtyRead(int tty_id, void *buf, int len);

/*------------------------------------------------- kernel_TtyWrite -----
|  Function kernel_TtyWrite
|
|  Purpose:  "writes" len chars from a input buffer by copying it's context (properly 
|           formatted) to the output buffer of the corresponding TTY
|           
|
|  Parameters:
|      int tty_id (IN) - id of the tty we write to
|      void *buf (IN) - pointer to the char array of the buffer we are
|                       writing from
|      int len (IN)    - length of the reading buffer
|
|  Returns:  length of the successful write, or ERROR
*-------------------------------------------------------------------*/
int kernel_TtyWrite(int tty_id, void *buf, int len);

#endif


#ifndef _GLOBALS_H_
#define _GLOBALS_H_
#include "list.h"
/* exit codes */
#define SUCCESS 0
#define FAILURE 1
#define ALLOCATION_ERROR (-3)
#define KILL 42
#define ERROR (-1)


/* macros */
//Checks if the alloc was sucessful.
#define ALLOC_CHECK(x, s){				\
	if(!x){						\
		puts("Memory allocation error: %s", s); \
		exit(ALLOCATION_ERROR);			\
	}						\
}






#endif 

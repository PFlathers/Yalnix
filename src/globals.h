
#ifndef _GLOBALS_H_
#define _GLOBALS_H_

/* exit codes */
#define FAILURE -1
#define ALLOCATION_ERROR -3


/* macros */
#define ALLOC_CHECK(x, s) if(!x){
	puts("Memory allocation error: %s", s);
	exit(ALLOCATION_ERROR);
}



#endif 
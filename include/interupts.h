#ifndef _INTERUPTS_H_
#define _INTERUPTS_H_
#include <hardware.h>

/*
 * Functions to be used for our kernel traps.
 * Interupt vector points to each of these.
 */
void trapKernel(UserContext *);
void trapClock(UserContext *);
void trapIllegal(UserContext *);
void trapMemory(UserContext *);
void trapMath(UserContext *);
void trapTTYReceive(UserContext *);
void trapTTYTransmit(UserContext *);


/*
 * we need to add dummy fucntions for the other 8 unused traps so our kernel
 * will not segflaut if we end up using them.
 */
void trapname1(UserContext *);
void trapname2(UserContext *);
void trapname3(UserContext *);
void trapname4(UserContext *);
void trapname5(UserContext *);
void trapname6(UserContext *);
void trapname7(UserContext *);
void trapname8(UserContext *);
void trapname9(UserContext *);

#endif

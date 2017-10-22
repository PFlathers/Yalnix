#include <stdlib.h>
#include <hardware.h>
#include "pcb.h"
#include "kernel.h"
#include "syscalls.h"

//Prototypes of kernel traps.
//This will be flushed out later.

void trapKernel(UserContext *uc)
{
}

void trapClock(UserContext *uc)
{
  void *addr;               // address of current process
  int clock_ticks;          // Number of clock ticks for delay
  int status;               // The exit status for delay
  int retval;

  switch (uc->code){
  	case YALNIX_DELAY:
  		clock_ticks = (int) uc->regs[0];
  		retval = kernel_Delay(uc, clock_ticks);
  		break;
  }


  uc->regs[0] = retval;
}

void trapIllegal(UserContext *uc)
{
}

void trapMemory(UserContext *uc)
{
}

void trapMath(UserContext *uc)
{
}

void trapTTYReceive(UserContext *uc)
{
}

void trapTTYTransmit(UserContext *uc)
{
}

void trapname1(UserContext *uc){}
void trapname2(UserContext *uc){}
void trapname3(UserContext *uc){}
void trapname4(UserContext *uc){}
void trapname5(UserContext *uc){}
void trapname6(UserContext *uc){}
void trapname7(UserContext *uc){}
void trapname8(UserContext *uc){}
void trapname9(UserContext *uc){}

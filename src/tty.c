#include "global.h"
#include "list.h"
#include "tty.h"
#include <string.h>


int TtyRead(int tty_id, void *buf, int len)
{
        if(!strcmp((char*)buf, "\n")){
                return 1;
        }
	return 0;
}

int TtyWrite(int tty_id, void *buf, int len)
{
	return 0;
}

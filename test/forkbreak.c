#include <stdio.h>

main(argc, argv)
int argc;
char *argv[];
{
    int pid;
    while(pid = Fork() != 0);
    TtyPrintf(0, "i am parent of pid: %d\n", pid);
    Wait(&pid);

    return 0;
}

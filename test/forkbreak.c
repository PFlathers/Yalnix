#include <stdio.h>

main(argc, argv)
int argc;
char *argv[];
{
    int pid;
    int i =0;

    for (i = 0; i<10; i++){
        pid = Fork();
        if (pid != 0){
                TtyPrintf(0, "i am parent of pid: %d\n", pid);
        }
    }


    // while(1){
    //     if (pid = Fork() != 0){
    //           TtyPrintf(0, "i am parent of pid: %d\n", pid);  
    //           Wait(&pid);
    //     }
    //     else{
    //            TtyPrintf(1, "child saying hi %d\n", pid);  
    //     }
    // }
    
    return 0;
}

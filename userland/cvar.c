#include <yalnix.h>
#include <string.h>

int main()
{

        
        char a [10];
        char b [10];
        strcpy(a, "init");
        strcpy(b, "\0");

        char *args[] = {a,b};
        //TracePrintf("%d\n", i);
        
	int i = Fork();
        int j, k;
        int retval = LockInit(&j); 
        int retval2 = CvarInit(&k);

        //Acquire(i);
        
	if(i == 0){
                while (Acquire(j)  != 0);
                CvarWait(k, j);

                /*
		while(1){
                TracePrintf(0, "I is child");
                Exec("init", args);
                Pause();
                }
*/
		return 0;
	}
        else{
/*
                while(1){
                        Pause();
                }
*/
//                Wait(&i);
                CvarSignal(k);
        }


	//TracePrintf(0, "I is parent");
        Reclaim(j);
        Reclaim(k);
	
	return 0;
}

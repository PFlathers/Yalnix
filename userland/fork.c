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
        
	if(i == 0){
                Pause();
                Pause();
                Pause();
                Pause();
                Pause();
                Pause();
                Pause();
                Pause();
                Pause();
                Pause();
                Pause();
                Pause();
                Pause();
                Pause();
                Pause();
                Pause();
                Pause();
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
                Wait(&i);
        }


	//TracePrintf(0, "I is parent");
	
	return 0;
}

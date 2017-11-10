#include <yalnix.h>
#include <string.h>

int main()
{
	int i = Fork();

        char a [10];
        char b [10];
        strcpy(a, "init");
        strcpy(b, '\0');

        char *args[] = {a,b};
        TracePrintf("%d\n", i);
	if(i == 0){
		while(1){
                TracePrintf(0, "I is child");
                Exec("init", args);
                Pause();
                }
	}
        else{
                while(1){
                        Pause();
                }
        }


	TracePrintf(0, "I is parent");
	
	return 0;
}

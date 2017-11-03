#include <yalnix.h>
#include "test.h"

int main()
{
	int i = Fork();
        char* a = (char*) malloc(sizeof(char) *10);
        char* b = (char*) malloc(sizeof(char) *10);

        strcpy(a, "init");
        strcpy(b, '\0');

        char *args[] = {a,b};
        TracePrintf("%d\n", i);
        
	if(i == 0){
                TracePrintf(0, "I is child");
                return 0;
	}

//        Exec("init", args);
        //Wait(&i);

	TracePrintf(0, "I is parent");
	
	return 0;
}

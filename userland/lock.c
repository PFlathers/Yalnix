#include <yalnix.h>
#include <string.h>

int main(int argc, char *argv[]) 
{
        int i;
        int retval = LockInit(&i); 
        Acquire(i);


	int j = Fork();
        //char* a = (char*) malloc(sizeof(char) *10);
        //char* b = (char*) malloc(sizeof(char) *10);

        TracePrintf("%d\n", i);
	if(j == 0){
           while (Acquire(i) == 0 ){

           }
	}
        else{
                Release(i);
        }


	TracePrintf(0, "I is parent");
	
	return 0;
}

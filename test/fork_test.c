#include <yalnix.h>
#include "test.h"

int main()
{
//	int i = Fork();
        char *args[] = {"init", 0};
 //       TracePrintf("%d\n", i);
        /*
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
*/

	TracePrintf(0, "I is parent");
	
	return 0;
}

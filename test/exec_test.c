#include <yalnix.h>
#include <string.h>
#include "test.h"

int main()
{
        char *name = (char *) malloc(10 *sizeof(char));
        strcpy(name, "init");
        char *end = (char *) malloc(10 *sizeof(char));
        strcpy(end, '\0');
        char *args[] = {name,end};
        Exec("init", args);
	
	return 0;
}

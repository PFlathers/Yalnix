#include <yalnix.h>

int main()
{
        char *a = (char*) malloc(100);
        TracePrintf(0, "%s", a);
        return 0;
}

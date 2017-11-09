#include <yalnix.h>
#include <string.h>

int main(int argc, char *argv[]) 
{
        int i;
        int retval = LockInit(&i); 
        Acquire(i);
        Release(i);
        return 0;
}

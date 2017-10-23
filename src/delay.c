#include <yalnix.h>

int main(int argc, char *argv[]) {
    TracePrintf(1, "delay ### start proceesszzszsz\n");
    while (1) {
        TracePrintf(3, "Delay for 4 because Patrick's lucky number\n");
        Delay(5);
    }
    return 0;
}

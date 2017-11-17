#include <yalnix.h>
#include <string.h>


int main(int argc, char *argv[]) { 
  
  TracePrintf(1, "TTY.c ### Start \n");
  
  char buffer[] = "Bruno Rocks!";
  int size = sizeof(buffer);
  
  int rc;
  rc = Fork();
  if (rc == 0) {
    Pause();
    // write to terminal one
    TtyWrite(1, buffer, size);
    TracePrintf(3, "\t writing to  1.\n");
    return 0;
  } 
  else {
    char read_buffer[10];
    size = sizeof(read_buffer);
    // if this works I will cut my left leg off
    TtyRead(2, read_buffer, size);
    TracePrintf(3, "\t tty 1 read:  %s.\n", read_buffer);
  }
  
  TracePrintf(1, "TTY.c ### End \n");
  
  return 0;
} 

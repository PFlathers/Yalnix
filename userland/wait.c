
int main(int argc, char *argv[]) {
  TracePrintf(1, "\t===>In wait_short.c\n");

  int pid;
  int i;
  pid = GetPid();
  
  int rc = Fork();
  // kiddo
  if (rc == 0) {
    pid = GetPid();
    TracePrintf(1, "\t pid of the child %d\n", pid);
    for (i = 0; i < 5; i++) {
      Pause();
    }
    TracePrintf(1, "\t child: exiting with return value 42\n");
    Exit(42);
  
  } 
  else { 
    Pause();

    // wait call should return automatically
    int *stat_ptr = (int *) malloc(sizeof(int));
    *stat_ptr = 0;
    TracePrintf(1, "\t parent about to wait for child\n");
    rc = Wait(stat_ptr);
  }

  return(-1); 
}

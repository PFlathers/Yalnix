#include <yalnix.h>
#include <string.h>

int main(int argc, char *argv[]) 
{
        int id;
        int rc;

        if (PipeInit(&id) != 0){
                TracePrintf(1, "pipe test: pipe init failed\n");
        }
        else{
                TracePrintf(1, "pipe test: pipe init with id %d\n", id);
        }

        rc = Fork();
        Pause();
        if (rc == 0) {
                char buffer[5]; //= (char *)malloc(5 * sizeof(char));
                buffer[0] = 'b';
                buffer[1] = 'r';
                buffer[2] = 'u';
                buffer[3] = 'n';
                buffer[4] = '\0';

                char buffer2[5];// = (char *)malloc(5 * sizeof(char));
                buffer2[0] = 's';
                buffer2[1] = 'u';
                buffer2[2] = 'c';
                buffer2[3] = 'k';
                buffer2[4] = '\0';

                int written;


                Pause();
                written = PipeWrite(id, buffer, 6);
                Pause();

                Pause();
                written = PipeWrite(id, buffer2, 6);

                Pause();
                char to_read[100] ;//= (char *)malloc(6 * sizeof(char));
                int read;
                Pause();
                Pause();
                read = PipeRead(id, &to_read, 6);
                TracePrintf(1, "\t pipe test: read %d chars from the pipe: %s\n", read, to_read);

                Pause();
                return 0;
        } else {
                char to_read[100] ;//= (char *)malloc(6 * sizeof(char));
                int read;
                Pause();
                Pause();
                read = PipeRead(id, &to_read, 6);
                TracePrintf(1, "\t pipe test: read %d chars from the pipe: %s\n", read, to_read);
                
                Wait(&rc);
                Pause();
                return 0;
        }

return 0;
}

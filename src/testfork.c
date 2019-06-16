#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>

__pid_t spid[3];

int main(int argc, char *argv[])
{
    __pid_t pid;

    printf("-1 %d %d\n", getpid(), getppid());

    for(int i = 0; i < 3; ++i) {
        pid = fork();
        if(pid == 0) {
            // printf("%d %d %d\n", i, getpid(), getppid());
            // exit(0);
            
        } else {
            printf("heheh  %d %d %d\n", i, getpid(), getppid());
        }
    }

    printf("Hello world!\n");

    return 0;
}
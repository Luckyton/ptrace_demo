#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <syscall.h>

int main(int argc, char *argv[])
{
    __pid_t pid;

    pid = fork();
    if(pid < 0) {
        perror("fork");
    } else if(pid == 0) {
        // sleep(1);
        printf("I am son : %d, %d\n", getpid(), getppid());
    } else {
        sleep(1);
        printf("I am father : %d, %d\n", getpid(), getppid());
    }

    return 0;
}
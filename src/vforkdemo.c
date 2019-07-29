#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <syscall.h>

int main(int argc, char *argv[])
{
    switch (vfork())
    {
    case -1:
        perror("vfork");
        break;
    case 0:
        sleep(1);
        printf("I am son : %d\n", getpid());
        exit(EXIT_SUCCESS);
    default:
        printf("I am father : %d\n", getpid());
        exit(EXIT_SUCCESS);
    }

    return 0;
}
#include <stdio.h>
#include <unistd.h>
#include <syscall.h>
#include <sys/ptrace.h>
#include <sys/wait.h>
#include <sys/user.h>

int main(int argc, char *argv[])
{
    int status;
    pid_t pid;
    struct user_regs_struct regs;

    pid = fork();
    if(pid < 0) {
        perror("fork");
    } else if(pid == 0) {
        execv(argv[1], &argv[2]);
    } else {
        
    }

    return 0;
}
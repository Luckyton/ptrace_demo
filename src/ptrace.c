#include <stdio.h>
#include <sys/ptrace.h>
#include <syscall.h>
#include <sys/user.h>
#include <unistd.h>
#include <wait.h>
#include <signal.h>
#include <assert.h>

#define TRUE    1
#define FALSE   0
#define VAR_NUM 1

static void setup(int nvar, char **argv)
{
    pid_t spid[3], tpid, pid;
    int status;
    unsigned long long orig_rax;
    
    for(int i = 0; i < nvar; ++i) {
        pid = fork();
        if(pid < 0) {
            perror("fork");
        } else if(pid == 0) {
            execv(argv[1], argv + 1);
        } else {
            spid[i] = pid;
            ptrace(PTRACE_ATTACH, spid[i], NULL, NULL);
            pid = waitpid(spid[i], &status, WUNTRACED);
            assert(pid == spid[i]);
            ptrace(PTRACE_SETOPTIONS, spid[i], NULL,
                    PTRACE_O_TRACECLONE | PTRACE_O_TRACEEXEC | PTRACE_O_TRACEEXIT |
                    PTRACE_O_TRACEFORK | PTRACE_O_TRACEVFORK | PTRACE_O_TRACESYSGOOD | PTRACE_O_EXITKILL);
            ptrace(PTRACE_SYSCALL, spid[i], NULL, NULL);
            waitpid(spid[i], &status, WUNTRACED);
            orig_rax = ptrace(PTRACE_PEEKUSER, spid[i], 8 * 15, NULL);
            assert(orig_rax == __NR_execve);
        }
    }

    for(int i = 0; i < nvar; ++i) {
        ptrace(PTRACE_SYSCALL, spid[i], NULL, NULL);
    }
}

static void wait_for_procs()
{
    int status, sig;
    pid_t pid;
    struct user_regs_struct regs;

    while(TRUE)
    {
        pid = waitpid(-1, &status, WUNTRACED);
        sig = WSTOPSIG(status);

        if(WIFEXITED(status)) {
            break;
        } else if(WIFSTOPPED(status) && sig == (SIGTRAP | 0x80)) {
            ptrace(PTRACE_GETREGS, pid, NULL, &regs);
            // printf("%d was stopped at %llu\n", pid, regs.orig_rax);
        }
        ptrace(PTRACE_SYSCALL, pid, NULL, NULL);
    }
}

int main(int argc, char *argv[])
{

    setup(3, argv);
    wait_for_procs();

    return 0;
}
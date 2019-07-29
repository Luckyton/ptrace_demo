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

pid_t spid[3];

static void setup(char **argv)
{
    pid_t pid;
    int status;
    unsigned long long orig_rax;
    
    for(int i = 0; i < VAR_NUM; ++i) {
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
                    PTRACE_O_TRACEEXEC | PTRACE_O_TRACEEXIT |
                    PTRACE_O_TRACESYSGOOD | PTRACE_O_EXITKILL);
            ptrace(PTRACE_SYSCALL, spid[i], NULL, NULL);
            waitpid(spid[i], &status, WUNTRACED);
            orig_rax = ptrace(PTRACE_PEEKUSER, spid[i], 8 * 15, NULL);
            assert(orig_rax == __NR_execve);
        }
    }

    for(int i = 0; i < VAR_NUM; ++i) {
        ptrace(PTRACE_SYSCALL, spid[i], NULL, NULL);
    }
}

static void wait_for_procs()
{
    int status, sig;
    pid_t pid;
    struct user_regs_struct regs;

    // for(int count = 0; count < VAR_NUM; )
    int count = 0;
    while (TRUE)
    {
        // printf("%d\n", count);

        pid = waitpid(-1, &status, WUNTRACED);
        sig = WSTOPSIG(status);

        //count = (count + 1) % VAR_NUM;

        if(WIFEXITED(status)) {
            // break;
            ++ count;
            if(count == 2) {
                break;
            }
        } else if(WIFSTOPPED(status) && sig == (SIGTRAP | 0x80)) {
            ptrace(PTRACE_GETREGS, pid, NULL, &regs);
            printf("%d was stopped at %llu, fork : %d, vfork : %d, clone : %d\n", 
                        pid, regs.orig_rax, __NR_fork, __NR_vfork, __NR_clone);
        }
        /**
        if(count == 0) {
            // ptrace(PTRACE_SYSCALL, pid, NULL, NULL);
            for(int i = 0; i < VAR_NUM; ++i) {
                ptrace(PTRACE_SYSCALL, spid[i], NULL, NULL);
            }
        }
        **/
       ptrace(PTRACE_SYSCALL, spid[0], NULL, NULL);
    }
    
}

int main(int argc, char *argv[])
{
    setup(argv);
    wait_for_procs();

    return 0;
}
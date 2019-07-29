#include <stdio.h>
#include <wait.h>
#include <syscall.h>
#include <unistd.h>
#include <signal.h>
#include <sched.h>
#include <sys/user.h>
#include <sys/ptrace.h>

struct monitor
{
    __pid_t * pids;
    char ** argv;
};

int new_monitor(void *arg);
void wait_for_procs();

void printReg(struct user_regs_struct * regs)
{
    printf("rdi : %llu\n", regs->rdi);
    printf("rsi : %llu\n", regs->rsi);
    printf("rdx : %llu\n", regs->rdx);
    printf("r10 : %llu\n", regs->r10);
    printf("r8 : %llu\n", regs->r8);
    printf("r9 : %llu\n", regs->r9);
}

void handle_syscall(pid_t pid)
{
    static int count = 0;
    ++count;
    struct user_regs_struct regs;

    ptrace(PTRACE_GETREGS, pid, NULL, &regs);
    
    printf("%d %llu %d\n", pid, regs.orig_rax, count);

    /**
    if(regs.orig_rax == __NR_clone && (count & 1)) {
        printf("第二次到了sys_clone\n");
        printReg(&regs);
    }
    **/

    ptrace(PTRACE_SYSCALL, pid, 0, 0);
    
}

void new_proc(pid_t old_pid, pid_t new_pid)
{
    
}

void wait_for_procs()
{
    int status, signal, event;
    pid_t pid, new_pid;
    int count = 0;

    while (1)
    {
        pid = waitpid(-1, &status, __WALL);
        if(pid == -1) {
            break;
        }

        signal = WSTOPSIG(status);
        event = status >> 16;
        
        if(WIFEXITED(status)) {
            ++ count;
            printf("count : %d, pid : %d\n", count, pid);
            if(count > 1) {
                printf("一切结束： count : %d, pid : %d\n", count, pid);
                break;
            }
        } else if(WIFSTOPPED(status) && signal == (SIGTRAP | 0x80)) {
            handle_syscall(pid);
        } else if(signal == SIGTRAP && (event == PTRACE_EVENT_FORK ||
                event == PTRACE_EVENT_VFORK ||
                event == PTRACE_EVENT_CLONE)) {
                    
            ptrace(PTRACE_GETEVENTMSG, pid, 0, &new_pid);
            printf("新进程的进程号：%d\n", new_pid);
            /**
            ptrace(PTRACE_SETOPTIONS, new_pid, 0,
                    PTRACE_O_TRACESYSGOOD | PTRACE_O_TRACEFORK |
                    PTRACE_O_TRACECLONE | PTRACE_O_TRACEVFORK |
                    PTRACE_O_TRACEEXEC | PTRACE_O_EXITKILL);
            printf("attach the father : %ld\n", ptrace(PTRACE_ATTACH, pid, 0, 0));
            // pid_t pidd = waitpid(new_pid, NULL, WUNTRACED);
            // printf("拦到的是：%d\n", pidd); */
            new_proc(pid, new_pid);
            // printf("释放父进程");
            // printf("%ld\n", ptrace(PTRACE_DETACH, pid, NULL, NULL));
            // printf("attach父进程");
            // printf("%ld\n", ptrace(PTRACE_ATTACH, pid, NULL, NULL));
            // printf("启动父进程\n");
            // printf("%lu\n", ptrace(PTRACE_SYSCALL, pid, NULL, NULL));
            // ptrace(PTRACE_SYSCALL, new_pid, NULL, NULL);*/
        } else if(signal == SIGTRAP && event == PTRACE_EVENT_EXEC) {
            printf("catch the pid : %d\n", pid);
            ptrace(PTRACE_SYSCALL, pid, 0, 0);
        } else {
            ptrace(PTRACE_SYSCALL, pid, 0, 0);
        }

        // ptrace(PTRACE_SYSCALL, pid, 0, 0);
    }
    
}

int new_monitor(void * arg)
{
    int status;
    pid_t * son;
    struct monitor *mo;

    son = mo->pids;
    while (son)
    {
        ptrace(PTRACE_ATTACH, *son, NULL, NULL);

        waitpid(*son, &status, __WALL);

        ptrace(PTRACE_SETOPTIONS, *son, NULL, 
                PTRACE_O_TRACEFORK | PTRACE_O_TRACECLONE |
                PTRACE_O_TRACEVFORK | PTRACE_O_TRACESYSGOOD);

        ++son;
    }
    
    son = mo->pids;
    while (son)
    {
        /* code */
        ptrace(PTRACE_SYSCALL, *son++, NULL, NULL);
    }

    wait_for_procs();
    
}

void setup(char *argv[])
{
    pid_t pid;
    int status;

    pid = fork();

    if(pid < 0) {
        perror("fork");
    } else if(pid == 0) {
        execv(argv[1], argv + 1);
    } else {
        ptrace(PTRACE_ATTACH, pid, NULL, NULL);
        waitpid(pid, &status, WUNTRACED);

        ptrace(PTRACE_SETOPTIONS, pid, 0, 
            PTRACE_O_TRACEEXEC | PTRACE_O_TRACECLONE | PTRACE_O_TRACEFORK |
            PTRACE_O_TRACESYSGOOD | PTRACE_O_TRACEEXIT | PTRACE_O_TRACEVFORK);
        
        ptrace(PTRACE_SYSCALL, pid, 0, 0);
        waitpid(pid, &status, WUNTRACED);

        ptrace(PTRACE_SYSCALL, pid, 0, 0);
    }
    
}

int main(int argc, char *argv[])
{
    // struct monitor mo;

    // mo.argv = argv;

    // new_monitor(&mo);

    printf("monitor pid : %d\n", getpid());

    setup(argv);
    // printf("Are you ok?\n");
    wait_for_procs();

    return 0;
}
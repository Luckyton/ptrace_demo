#include <stdio.h>
#include <pthread.h>
#include <unistd.h>
#include <wait.h>
#include <stdlib.h>
#include <signal.h>
#include <syscall.h>
#include <sys/user.h>
#include <sys/ptrace.h>

#define TRUE 1
#define FALSE 0
#define VAR_NUM 3

pid_t pids[VAR_NUM];

static void setup(char **argv)
{
    int status;
    pid_t pid;
    unsigned long long orig_rax;

    for(int i = 0; i < VAR_NUM; ++i) {
        pids[i] = fork();
        if(pids[i] < 0) {
            perror("fork error");
        } else if(pids[i] == 0) {
            execv(argv[0], argv);
        } else {
            ptrace(PTRACE_ATTACH, pids[i], NULL, NULL);
            pid = waitpid(pids[i], &status, WUNTRACED);
            if(pid != pids[i]) {
                perror("pid not queal");
            }
            ptrace(PTRACE_SETOPTIONS, pids[i], NULL,
                PTRACE_O_TRACECLONE | PTRACE_O_TRACEEXEC | PTRACE_O_TRACEEXIT |
                PTRACE_O_TRACEFORK | PTRACE_O_TRACEVFORK | PTRACE_O_TRACESYSGOOD | PTRACE_O_EXITKILL);
            ptrace(PTRACE_SYSCALL, pids[i], NULL, NULL);
            waitpid(pids[i], &status, WUNTRACED);
            orig_rax = ptrace(PTRACE_PEEKUSER, pids[i], 8 * 15, NULL);
            if(orig_rax != SYS_execve) {
                perror("not execve");
            }
        }
    }
    for(int i = 0; i < VAR_NUM; ++i) {
        ptrace(PTRACE_SYSCALL, pids[i], NULL, NULL);
    }
}

static void wait_for_procs()
{
    // pid_t pid;
    // int count, sig, event, status;
    // struct user_regs_struct regs;

    // count = 0;
    // while(TRUE)
    // {
    //     pid = waitpid(-1, &status, WUNTRACED);
    //     sig = WSTOPSIG(status);
    //     event = status >> 16;

    //     count = (count + 1) % VAR_NUM;
        
    //     if(WIFEXITED(status)) {
    //         if(count == 0) {
    //             break;
    //         }
    //     } else if(WIFSTOPPED(status) && sig == (SIGTRAP | 0x80)) {
    //         // ptrace(PTRACE_GETREGS, pid, 0, &regs);
    //         // printf("%d was stopped at %llu\n", pid, regs.orig_rax);
            
    //     }

    //     if(count == 0) {
    //         if(sig)
    //         // ptrace(PTRACE_GETREGS, pid, 0, &regs);
    //         for(int i = 0; i < VAR_NUM; ++i) {
    //             ptrace(PTRACE_SYSCALL, pids[i], NULL, NULL);
    //         }
    //     }
    // }

    pid_t orphans[128] = { 0 };
    int status, event, sig;
    pid_t pid;

    int count = 0;
    while (1)
    {
        if ((pid = waitpid(-1, &status, __WALL)) == -1)
            break;

        event = status >> 16;
        sig = WSTOPSIG(status);

        count = (count + 1) % (2 * VAR_NUM);

        if (WIFSTOPPED(status) && sig == (SIGTRAP | 0x80))
        {
            // handle_syscall(pid, 1);
        }
        else if (WIFEXITED(status))
        {
            if(count % VAR_NUM == 0) {
                break;
            }
            // mv_thread_t thread = mv_thread_get(pid);
            // debug_print("%d exited normally\n", pid);
            // check_wakeups_vars(thread);
            // check_wakeups_threads(thread);
            // mv_thread_exit(thread);
        }
        else if (WIFSTOPPED(status) && sig == SIGSTOP)
        {
            unsigned i;
            // debug_print("%d SIGSTOP\n", pid);

            /* Sometimes we get the sigstop before the FORK event of newly
             * created children. */
            for (i = 0; i < 128 && orphans[i]; i++);
            // assert(i < 128 - 2);
            orphans[i] = pid;
            orphans[i + 1] = 0;

            //ptrace(PTRACE_SYSCALL, pid, NULL, NULL);
            /* TODO: cont proc if not the inital SIGSTOP */
        }
        else if (sig == SIGTRAP && event == PTRACE_EVENT_EXEC)
        {
            // ptrace(PTRACE_SYSCALL, pid, NULL, NULL);
            if(count % VAR_NUM == 0) {
                for(int i = 0; i < VAR_NUM; ++i) {
                    ptrace(PTRACE_SYSCALL, pids[i], NULL, NULL);
                }
            }
        }
        else if (sig == SIGTRAP &&
                (event == PTRACE_EVENT_FORK ||
                 event == PTRACE_EVENT_VFORK ||
                 event == PTRACE_EVENT_CLONE))
        {
            struct user_regs_struct regs;
            unsigned i;
            int orphan_index = -1;
            pid_t new_pid, vtid;
            // mv_thread_t thread, new_thread;
            // thread = mv_thread_get(pid);
            ptrace(PTRACE_GETEVENTMSG, pid, 0, &new_pid);
            ptrace(PTRACE_GETREGS, pid, NULL, &regs);

            /* If this is not an orphan we will wait for a SIGSTOP explicitly
             * here so we know the process has started. If it is an orphan, we
             * recceived this SIGSTOP earlier, before the EVENT_FORK. */
            for (i = 0; i < 128 && orphans[i]; i++)
                if (orphans[i] == new_pid)
                    orphan_index = i;

            // if (orphan_index != -1)
            // {
            //     for (i = 0; i < 128 && orphans[i]; i++);
            //     orphans[orphan_index] = orphans[i - 1];
            //     orphans[i - 1] = 0;
            // }
            // else
            // {
            //     waitpid(new_pid, &status, __WALL);
            //     assert(WIFSTOPPED(status));
            // }


            // if (regs.rdi & CLONE_THREAD)
            //     new_thread = mv_thread_new(new_pid, mv_thread_getpgid(thread), pid);
            // else
            // {
            //     mv_proc_new(new_pid, pid, -1);
            //     new_thread = mv_thread_new(new_pid, new_pid, -1);
            // }

            // if (regs.rdi & CLONE_CHILD_SETTID)
            // {
            //     vtid = mv_thread_getvtid(new_thread);
            //     copy_to_user(new_pid, (void *)regs.r10, &vtid, sizeof(pid_t));
            // }
            // if (regs.rdi & CLONE_PARENT_SETTID)
            // {
            //     vtid = mv_thread_getvtid(new_thread);
            //     copy_to_user(new_pid, (void *)regs.rdx, &vtid, sizeof(pid_t));
            // }

            // debug_print("%d forked into %d\n", pid, new_pid);
            ptrace(PTRACE_SETOPTIONS, new_pid, NULL,
                    PTRACE_O_TRACESYSGOOD | PTRACE_O_TRACEFORK |
                    PTRACE_O_TRACECLONE | PTRACE_O_TRACEVFORK |
                    PTRACE_O_TRACEEXEC);
            ptrace(PTRACE_SYSCALL, new_pid, NULL, NULL);
            ptrace(PTRACE_SYSCALL, pid, NULL, NULL);
        }
        else if (WIFSTOPPED(status))
        {
            // debug_print("%d signal %d\n", pid, sig);
            ptrace(PTRACE_SYSCALL, pid, NULL, sig);
        }
        else if (WIFSIGNALED(status))
        {
            // mv_thread_t thread = mv_thread_get(pid);
            // fprintf(stderr, "%d got terminated by signal %s\n", pid,
            //         strsignal(WTERMSIG(status)));
            // check_wakeups_vars(thread);
            // check_wakeups_threads(thread);
            // mv_thread_exit(thread);
        }
        else
        {
            // fprintf(stderr, "%d stopped due to %d\n", pid, status);
            // fprintf(stderr, "%d ifstopped: %d\n", pid, WIFSTOPPED(status));
            // fprintf(stderr, "%d ifsignalled: %d\n", pid, WIFSIGNALED(status));
            // fprintf(stderr, "%d term sig: %s (%d)\n", pid, strsignal(WTERMSIG(status)), WTERMSIG(status));
            // fprintf(stderr, "%d stopping sig: %s (%d)\n", pid, strsignal(sig), sig);
            // print_backtrace(pid);
            break;
        }
    }
}

int main(int argc, char *argv[])
{
    setup(argv + 1);
    wait_for_procs();

    return 0;
}
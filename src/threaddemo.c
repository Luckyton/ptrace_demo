#include <stdio.h>
#include <pthread.h>

static void *
testFunc(void * p_count)
{
    printf("我是子线程\n");

    int count = *(int *)p_count;
    FILE * p_file;

    p_file = fopen("debug.log", "a+");

    for(int i = 0; i < count; ++i) {
        fprintf(p_file, "%d\n", i);
    }
}

int main(int argc, char *argv[])
{
    pthread_t pt;

    int count = 8;

    if(pthread_create(&pt, NULL, testFunc, (void *)(&count)) != 0) {
        printf("Hello\n");
        perror("pthread");
    }

    pthread_join(pt, NULL);
    printf("ok\n");

    pthread_exit(0);
}
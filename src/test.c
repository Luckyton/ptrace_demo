#include <stdio.h>

int main(int argc, char *argv[])
{
    /**
    char **p_arg;
    p_arg = argv;
    while (*p_arg)
    {
        printf("%s\n", *p_arg);
    }
    */

    printf("%s\n", argv[1]);
    
    return 0;
}
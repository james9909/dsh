#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include "builtins.h"

int main()
{
    char buf[512] = {};
    char *argl[512] = {};
    char *args = buf;
    char *arg;
    printf("%% ");
    while (fgets(buf, sizeof(buf), stdin))
    {
        args = buf;
        buf[strlen(buf)-1] = 0;
        int i = 0;
        for (i = 0; args; ++i)
        {
            arg = strsep(&args, " ");
            argl[i] = arg;
        }
        argl[i] = 0;

        handle_builtins(argl);

        pid_t n = fork();
        if (n == 0)
        {
            execvp(argl[0], argl);
        }
        int status;
        wait(&status);
        printf("%% ");
    }
    return 0;
}

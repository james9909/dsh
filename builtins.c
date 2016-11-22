#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

#include "builtins.h"

int handle_builtins(char* argl[])
{
    if (strcmp(argl[0], "exit") == 0)
    {
        exit(0);
        return 1;
    }
    if (strcmp(argl[0], "cd") == 0)
    {
        chdir(argl[1]);
        return 1;
    }
    return 0;
}

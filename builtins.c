#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include "builtins.h"

void handle_builtins(char* argl[])
{
    if (strcmp(argl[0], "exit") == 0)
    {
        exit(0);
    }
    if (strcmp(argl[0], "cd") == 0)
    {
        chdir(argl[1]);
    }
}

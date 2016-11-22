#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <wordexp.h>

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
        char *path;
        if (sizeof(&argl) / sizeof(char *) == 1) {
            path = "~";
        } else {
            path = argl[1];
        }

        int status = chdir(expand(path));
        if (status == -1) {
            printf("cd: %s: %s\n", strerror(errno), path);
        }
        return 1;
    }
    return 0;
}

char *expand(char *path) {
    wordexp_t expanded;
    wordexp(path, &expanded, 0);
    return expanded.we_wordv[0];
}

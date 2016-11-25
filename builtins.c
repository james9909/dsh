#include <glob.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <wordexp.h>

#include "builtins.h"

int handle_builtins(char **argl)
{
    if (strcmp(argl[0], "exit") == 0)
    {
        exit(0);
        return 1;
    }
    if (strcmp(argl[0], "cd") == 0)
    {
        char *path;

        int argc;
        for (argc = 0; argl[argc + 1]; argc++);

        if (argc == 0) {
            path = "~";
        } else if (argc == 1) {
            path = argl[1];
        } else {
            printf("cd: Too many arguments\n");
            return 1;
        }

        int status = chdir(path);
        if (status == -1) {
            printf("cd: %s: %s\n", strerror(errno), path);
        }
        return 1;
    }
    return 0;
}

char **expand(char **argl) {
    glob_t globbuf;
    int i, j;
    int flags = GLOB_TILDE;// | GLOB_NOCHECK;

    int size = 128;
    char **new = (char **) calloc(size, sizeof(char*));
    int newi = 0;

    for (i = 0; argl[i] != NULL; i++) {
        int result = glob(argl[i], flags , NULL, &globbuf);
        if (result == GLOB_NOMATCH) {
            new[newi++] = argl[i];
            continue;
        }
        for (j = 0; j < globbuf.gl_pathc; ++j) {
            new[newi++] = strdup(globbuf.gl_pathv[j]);
            if (newi > size) {
                new = realloc(new, size*2);
            }
        }
    }
    globfree(&globbuf);
    return new;
}

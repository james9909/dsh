#include <glob.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <wordexp.h>
#include <assert.h>

#include "command.h"
#include "executor.h"
#include "builtins.h"
#include "aliases.h"

void single_expand(char **arg)
{
    glob_t globbuf;
    glob(*arg, GLOB_TILDE|GLOB_NOCHECK, NULL, &globbuf);
    *arg = strdup(globbuf.gl_pathv[0]);
    globfree(&globbuf);
}

int is_builtin(Command *c)
{
    char *cmd = c->argv[0];
    if (strcmp(cmd, "cd") == 0
     || strcmp(cmd, "exit") == 0
     || strcmp(cmd, "alias") == 0)
        return 1;
    if (c->argc > 1 && strcmp(c->argv[1], "=") == 0)
        return 1;
    return 0;
}

void handle_builtins(Command *c)
{
    if (strcmp(c->argv[0], "exit") == 0)
    {
        int exit_code = 0;
        if (c->argc > 1)
        {
            exit_code = atoi(c->argv[1]);
        }
        exit(exit_code);
    }
    if (strcmp(c->argv[0], "cd") == 0)
    {
        char *path;

        if (c->argc == 1) {
            path = "~";
        } else if (c->argc == 2) {
            path = c->argv[1];
        } else {
            printf("cd: Too many arguments\n");
            return;
        }
        single_expand(&path);

        int status = chdir(path);
        if (status == -1) {
            printf("cd: %s: %s\n", strerror(errno), path);
        }
        free(path);
        return;
    }
    if (strcmp(c->argv[0], "alias") == 0)
    {
        if (c->argc < 4)
        {
            fprintf(stderr, "alias: Syntax error.\n");
            exit(1);
        }
        add_alias(c->argv[1], c->argv[3]);
        return;
    }
    if (c->argc > 1 && strcmp(c->argv[1], "=") == 0)
    {
        if (c->argc < 3)
        {
            fprintf(stderr, "Syntax error.\n");
            exit(1);
        }
        setenv(c->argv[0], c->argv[2], 1);
        return;
    }
    assert(0); //if triggered, is_builtin is not right
}

char **expand(char **argl) {
    glob_t globbuf;
    int i, j;
    int flags = GLOB_TILDE | GLOB_NOCHECK;

    int size = 128;
    char **new = (char **) calloc(size, sizeof(char*));
    int newi = 0;

    for (i = 0; argl[i] != NULL; i++)
    {
        glob(argl[i], flags , NULL, &globbuf);
        for (j = 0; j < globbuf.gl_pathc; ++j)
        {
            new[newi++] = strdup(globbuf.gl_pathv[j]);
            if (newi > size)
            {
                new = realloc(new, size*2);
            }
        }
    }
    globfree(&globbuf);
    return new;
}

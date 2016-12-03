#include <glob.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <wordexp.h>
#include <assert.h>
#include <signal.h>
#include <wait.h>

#include "command.h"
#include "executor.h"
#include "builtins.h"
#include "aliases.h"

void expand_path(char **arg)
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
     || strcmp(cmd, "alias") == 0
     || strcmp(cmd, "fg") == 0)
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

        if (c->argc == 1)
        {
            path = "~";
        }
        else if (c->argc == 2)
        {
            path = c->argv[1];
        }
        else
        {
            printf("cd: Too many arguments\n");
            return;
        }
        expand_path(&path);

        int status = chdir(path);
        if (status == -1)
        {
            printf("cd: %s: %s\n", strerror(errno), path);
        }
        free(path);
        return;
    }
    if (strcmp(c->argv[0], "fg") == 0)
    {
        signal_process(SIGCONT);
        int status;
        waitpid(get_pid(), &status, 0);
        return;
    }
    if (strcmp(c->argv[0], "alias") == 0)
    {
        if (c->argc < 4)
        {
            fprintf(stderr, "alias: Syntax error.\n");
            return;
        }
        add_alias(c->argv[1], c->argv[3]);
        return;
    }
    if (c->argc > 1 && strcmp(c->argv[1], "=") == 0)
    {
        if (c->argc < 3)
        {
            fprintf(stderr, "Syntax error.\n");
            return;
        }
        setenv(c->argv[0], c->argv[2], 1);
        return;
    }
    assert(0); //if triggered, is_builtin is not right
}

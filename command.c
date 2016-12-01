#include <glob.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>

#include "command.h"
#include "prompt.h"

void handle_redirects(Command *c)
{
    if (*c->stdin_redir_f)
    {
        int fd = open(c->stdin_redir_f, O_RDONLY);
        dup2(fd, STDIN_FILENO);
        close(fd);
    }
    if (c->stdout_redir)
    {
        dup2(c->stdout_redir, STDOUT_FILENO);
        close(c->stdout_redir);
    }
    if (*c->stdout_redir_f)
    {
        int append = c->stdout_append ? O_APPEND : 0;
        int fd = open(c->stdout_redir_f, O_WRONLY|O_CREAT|append, 0644);
        dup2(fd, STDOUT_FILENO);
        close(fd);
    }
    if (c->stderr_redir)
    {
        dup2(c->stderr_redir, STDERR_FILENO);
        close(c->stderr_redir);
    }
    if (*c->stderr_redir_f)
    {
        int append = c->stderr_append ? O_APPEND : 0;
        int fd = open(c->stderr_redir_f, O_WRONLY|O_CREAT|append, 0644);
        dup2(fd, STDERR_FILENO);
        close(fd);
    }
}

Command *next_cmd(Command *c)
{
    if (c->pipe_to)
        return c->pipe_to;
    else if (c->and_to)
    {
        if (get_exit_code() == 0)
            return c->and_to;
        else
            return next_cmd(c->and_to);
    }
    else if (c->or_to)
    {
        if (get_exit_code() == 0)
            return next_cmd(c->or_to);
        else
            return c->or_to;
    }
    else if (c->next_cmd)
        return c->next_cmd;
    else
        return NULL;
}

void expand(Command *c)
{
    glob_t globbuf;
    int i, j;
    int argc = 0;
    int flags = GLOB_TILDE | GLOB_NOCHECK;
    char **copy = (char **) calloc(sizeof(c->argv), sizeof(char *));
    memcpy(copy, c->argv, sizeof(c->argv));

    for (i = 0; copy[i]; ++i)
    {
        glob(copy[i], flags, NULL, &globbuf);
        for (j = 0; j < globbuf.gl_pathc; ++j)
        {
            c->argv[argc++] = strdup(globbuf.gl_pathv[j]);
        }
        globfree(&globbuf);
    }
    c->argc = argc;

    free(copy);
}

void handle_pipes(Command *c)
{
    if (c->pipe_out)
    {
        dup2(c->pipe_out, STDOUT_FILENO);
    }
    if (c->pipe_in)
    {
        dup2(c->pipe_in, STDIN_FILENO);
    }
}

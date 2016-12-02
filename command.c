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

void clear_cmd(Command *c)
{
    c->argc = 0;
    c->stdout_append = 0;
    c->stderr_append = 0;
    c->stdout_redir = 0;
    c->stderr_redir = 0;
    memset(c->stdin_redir_f, 0, sizeof(c->stdin_redir_f));
    memset(c->stdout_redir_f, 0, sizeof(c->stdout_redir_f));
    memset(c->stderr_redir_f, 0, sizeof(c->stderr_redir_f));
    c->pipe_in = 0;
    c->pipe_out = 0;
    c->pipe_to = 0;
    c->piped_from = 0;
    c->and_to = 0;
    c->and_from = 0;
    c->or_to = 0;
    c->or_from = 0;
    c->next_cmd = 0;
    c->prev_cmd = 0;
}

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

void free_cmds(Command *c)
{
    if (c == NULL)
        return;
    free_cmds(c->pipe_to);
    free_cmds(c->and_to);
    free_cmds(c->or_to);
    free_cmds(c->next_cmd);
    int i;
    for (i = 0; i < c->argc; ++i)
    {
        free(c->argv[i]);
    }
    free(c);
}

void print_cmd(Command *c)
{
    printf("Command at %p\n", c);
    int i;
    for (i = 0; i < c->argc; ++i)
    {
        printf("argv[%d]: %s\n", i, c->argv[i]);
    }
    printf("argc: %d\n", c->argc);
    printf("stdin_redir_f: %s\n"
           "stdout_redir: %d\n"
           "stderr_redir: %d\n"
           "stdout_redir_f: %s\n"
           "stderr_redir_f: %s\n"
           "stdout_append: %d\n"
           "stderr_append: %d\n",
           c->stdin_redir_f, c->stdout_redir, c->stderr_redir,
           c->stdout_redir_f, c->stderr_redir_f,
           c->stdout_append, c->stderr_append);
    printf("Pipes to: %p\n", c->pipe_to);
    printf("Piped from: %p\n", c->piped_from);
    printf("And to: %p\n", c->and_to);
    printf("And from: %p\n", c->and_from);
    printf("Or to: %p\n", c->or_to);
    printf("Or from: %p\n", c->or_from);
    printf("Next cmd: %p\n", c->next_cmd);
    printf("Prev cmd: %p\n", c->prev_cmd);
    printf("-----------------------------\n");
}

void print_cmds(Command *c)
{
    if (c == NULL)
        return;
    print_cmd(c);
    print_cmds(c->pipe_to);
    print_cmds(c->and_to);
    print_cmds(c->or_to);
    print_cmds(c->next_cmd);
}

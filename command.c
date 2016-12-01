#include <stdio.h>
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

Command *next_cmd(Command *c) {
    if (c->pipe_to) {
        return c->pipe_to;
    } else if (c->and_to) {
        if (get_exit_code() == 0) {
            return c->and_to;
        } else {
            return next_cmd(c->and_to);
        }
    } else if (c->or_to) {
        if (get_exit_code() == 0) {
            return next_cmd(c->or_to);
        } else {
            return c->or_to;
        }
    } else if (c->next_cmd) {
        return c->next_cmd;
    } else {
        return NULL;
    }
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

#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>

#include "command.h"

void handle_redirects(Command *c)
{
    if (c->stdin_redir_f)
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
    if (c->stdout_redir_f)
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
    if (c->stderr_redir_f)
    {
        int append = c->stderr_append ? O_APPEND : 0;
        int fd = open(c->stderr_redir_f, O_WRONLY|O_CREAT|append, 0644);
        dup2(fd, STDERR_FILENO);
        close(fd);
    }
}

Command *get_piped(Command *c)
{
    return c->pipe_to;
}
Command *get_and(Command *c)
{
    return c->and_to;
}
Command *get_or(Command *c)
{
    return c->or_to;
}
Command *get_next(Command *c)
{
    return c->next_cmd;
}

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <fcntl.h>

#include "command.h"
#include "prompt.h"
#include "executor.h"
#include "builtins.h"
#include "aliases.h"

pid_t pid = -1;

void exec(Command *c)
{
    if (c->argv[0] == NULL)
    {
        return;
    }
    expand(c);

    handle_aliases(c);
    if (is_builtin(c))
    {
        handle_builtins(c);
    }
    else
    {
        pid = fork();
        if (pid == 0) //child
        {
            handle_redirects(c);
            handle_pipes(c);
            execvp(c->argv[0], c->argv);
            fprintf(stderr, "dsh: Command not found: %s\n", c->argv[0]);
            exit(1);
        }

        int exit_code;
        waitpid(pid, &exit_code, 0);

        if (c->pipe_in)
        {
            close(c->pipe_in);
            c->pipe_in = 0;
        }
        if (c->pipe_out)
        {
            close(c->pipe_out);
            c->pipe_out = 0;
        }

        if (WIFEXITED(exit_code))
        {
            set_exit_code(WEXITSTATUS(exit_code));
        }
        pid = -1;
    }
}

void run(Command *c)
{
    if (c == NULL)
        return;

    Command *i = c;
    while (i)
    {
        if (i->pipe_to)
        {
            int pfds[2];
            pipe(pfds);
            i->pipe_out = pfds[1];
            i->pipe_to->pipe_in = pfds[0];
        }
        exec(i);
        i = next_cmd(i);
    }
}

void signal_process(int signo)
{
    if (pid > 0)
    {
        kill(pid, signo);
    }
}

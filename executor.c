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

int get_pid()
{
    return pid;
}

int exec(Command *c)
{
    if (c->argv[0] == NULL)
    {
        return 0;
    }
    expand(c);

    handle_aliases(c);
    //print_cmds(c);
    if (c->abort)
    {
        fprintf(stderr, "dsh: Aborting...\n");
        return 0;
    }
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

        if (!c->dont_wait)
        {
            int exit_code;
            waitpid(pid, &exit_code, 0);
            if (WIFEXITED(exit_code))
            {
                set_exit_code(WEXITSTATUS(exit_code));
            }
            pid = -1;
        }
        else
        {
            int status;
            waitpid(-1, &status, WNOHANG);
        }

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

    }
    return 1;
}

void run(Command *c)
{
    if (c == NULL)
        return;

    int valid = 1;
    Command *i = c;
    while (i && valid)
    {
        if (i->pipe_to)
        {
            int pfds[2];
            pipe(pfds);
            i->pipe_out = pfds[1];
            i->pipe_to->pipe_in = pfds[0];
        }
        if (i->dont_wait)
        {
            apply_dont_wait(i);
        }
        valid = exec(i);
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

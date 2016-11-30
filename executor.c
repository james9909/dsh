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
    if (is_builtin(c->argv[0]))
    {
        handle_builtins(c);
    }
    else
    {
        pid = fork();
        if (pid == 0) //child
        {
            /* handle_aliases(c); */
            handle_redirects(c);
            /* handle_pipes(c); */
            execvp(c->argv[0], c->argv);
            fprintf(stderr, "dsh: Command not found: %s\n", c->argv[0]);
            exit(1);
        }

        int exit_code;
        waitpid(pid, &exit_code, 0);

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
        exec(i);
        int n = 0;
        if (i->pipe_to)
        {
            i = i->pipe_to;
            n++;
        }
        if (i->and_to)
        {
            i = i->and_to;
            n++;
        }
        if (i->or_to)
        {
            i = i->or_to;
            n++;
        }
        if (i->next_cmd)
        {
            i = i->next_cmd;
            n++;
        }
        if (n == 0)
            i = 0;
        if (n > 1)
        {
            fprintf(stderr, "n > 1\nURGENT\n");
        }
    }
}

void signal_process(int signo)
{
    if (pid > 0)
    {
        kill(pid, signo);
    }
}

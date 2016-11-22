#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <fcntl.h>

#include "prompt.h"
#include "executor.h"
#include "builtins.h"

pid_t pid;

void remove_spaces(char *argv[512])
{
    int i;
    for (i = 0; argv[i]; ++i)
    {
        if (strcmp(argv[i], " ") == 0)
        {
            if (!argv[i+1])
            {
                argv[i] = 0;
                continue;
            }
            int j;
            for (j = i+1; argv[j]; ++j)
            {
                argv[j-1] = argv[j];
            }
            argv[j] = 0;
        }
    }
}

void handle_redirect(char *argv[512])
{
    int i;
    for (i = 0; argv[i]; ++i)
    {
        char *p = argv[i];
        //Redirect stdout
        if (p[0] == '>' ||
           (strlen(p) > 2 && p[1] == '>'))
        {
            int src;
            int append = 0;
            int outerr = 0;
            if (p[0] == '>')
            {
                src = 1;
                p++;
            }
            else
            {
                if (p[0] == '&')
                    outerr = 1;
                else
                    src = p[0] - '0';
                p++;
                p++;
            }
            int fd;
            if (p[1] == '>')
            {
                append = O_APPEND;
                p++;
            }
            //Redirect to stdin/stdout/stderr
            if (p[0] == '&')
            {
                p++;
                fd = p[0] - '0';
                if (fd > 2 || p[1] != '\0')
                {
                    perror("dsh: Bad file descriptor");
                    exit(1);
                }
                dup2(fd, src);
                close(fd);
            } else //redirect to file
            {
                fd = open(p, O_WRONLY|O_CREAT|append, 0644);
                dup2(fd, src);
                close(fd);
            }
            argv[i] = " ";
        }
    }
}

void handle_pipes(char *cmd, int num_pipes)
{
    char *pipe_cmds[512];
    int pfds[2*num_pipes];
    int i = 0;
    for (i = 0; i < num_pipes; ++i)
    {
        pipe(pfds + i*2);
    }

    for (i = 0; pipe_cmds[i] = strsep(&cmd, "|"); ++i)
    {
        //clear command of spaces from both ends
        while (pipe_cmds[i][0] == ' ') pipe_cmds[i]++;
        int j;
        for (j = strlen(pipe_cmds[i]) - 1; pipe_cmds[i][j] == ' '; --j)
        {
            pipe_cmds[i][j] = 0;
        }
    }
    char *argv[512];
    for (i = 0; pipe_cmds[i]; ++i)
    {
        //handle_builtins(&pipe_cmds[i]);
        pid = fork();
        if (pid == 0)
        {
            if (i != 0)
                dup2(pfds[(i-1)*2], 0);
            if (i != num_pipes)
                dup2(pfds[i*2+1], 1);
            int j;
            for (j = 0; pipe_cmds[i]; ++j)
                argv[j] = strsep(&pipe_cmds[i], " ");
            handle_redirect(argv);
            remove_spaces(argv);
            execvp(argv[0], argv);
            printf("dsh: Command not found: %s\n", argv[0]);
            exit(127);
        }
        if (i != 0)
            close(pfds[(i-1)*2]);
        if (i != num_pipes)
            close(pfds[i*2+1]);

        int exit_code;
        waitpid(pid, &exit_code, 0);

        if (WIFEXITED(exit_code))
        {
            set_exit_code(WEXITSTATUS(exit_code));
        }
        pid = -1;
    }
}

void run(char *input)
{
    input[strlen(input)-1] = 0;

    char *cmds[512];
    char *argv[512] = {};
    int i,j;
    for (i = 0; cmds[i] = strsep(&input, ";"); ++i)
    {
        while (cmds[i][0] == ' ') cmds[i]++;
        for (j = strlen(cmds[i])-1; cmds[i][j] == ' '; --j)
            cmds[i][j] = 0;

        char *cmd = cmds[i];
        int num_pipes = 0;
        int k = 0;
        for (k = 0; cmd[k]; ++k)
        {
            if (cmd[k] == '|') num_pipes++;
        }

        if (num_pipes > 0)
        {
            handle_pipes(cmd, num_pipes);
            continue;
        }

        for (j = 0; cmds[i]; ++j)
        {
            argv[j] = strsep(&cmds[i], " ");
        }
        argv[j] = 0;

        if (strlen(*argv) == 0)
        {
            continue;
        }

        if (handle_builtins(argv))
            continue;

        pid = fork();
        if (pid == 0)
        {
            handle_redirect(argv);
            remove_spaces(argv);
            execvp(argv[0], argv);
            printf("dsh: Command not found: %s\n", argv[0]);
            exit(127);
        } else
        {
            int exit_code;
            waitpid(pid, &exit_code, 0);

            if (WIFEXITED(exit_code))
            {
                set_exit_code(WEXITSTATUS(exit_code));
            }
            pid = -1;
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

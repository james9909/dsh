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

/*
 * We allocate memory only in the child process.
 * The process image is immediately replaced
 * by execvp. It's both unnecessary and impossible
 * to free, as we need to provide those arguments
 * to execvp. If it fails, we immediately exit,
 * again making freeing useless.
 */

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
            for (j = i; argv[j+1]; ++j)
            {
                argv[j] = argv[j+1];
            }
            argv[j] = 0;
            i--;
        }
    }
}

void combine_quoted(char *argv[512])
{
    int dquote = 0;
    int squote = 0;
    int dquote_args[512] = {};
    int squote_args[512] = {};
    int di = 0;
    int si = 0;

    int i;
    for (i = 0; argv[i]; ++i)
    {
        char *p = argv[i];
        if (p[0] == '"')
        {
            dquote_args[di++] = i;
            argv[i]++;
        }
        if (p[0] == '\'')
        {
            squote_args[si++] = i;
            argv[i]++;
        }
        if (p[strlen(p)-1] == '"')
        {
            if (di == 0)
            {
                perror("dsh: Ill-formed string");
                exit(1);
            }
            p[strlen(p)-1] = 0;
            int start_quote = dquote_args[di-1];
            if (i != start_quote)
            {
                int size = i - start_quote;
                int j;
                for (j = start_quote; j <= i; ++j)
                {
                    size += strlen(argv[j]);
                }
                char *tmp = (char*)malloc(size*sizeof(char));
                strcpy(tmp, argv[start_quote]);
                free(argv[start_quote]);
                argv[start_quote] = tmp;
                for (j = start_quote+1; j <= i; ++j)
                {
                    strcat(argv[start_quote], " ");
                    strcat(argv[start_quote], argv[j]);
                    argv[j] = " ";
                }
                dquote_args[--di] = -1;
            }
        }
        if (p[strlen(p)-1] == '\'')
        {
            if (si == 0)
            {
                perror("dsh: Ill-formed string");
                exit(1);
            }
            p[strlen(p)-1] = 0;
            int start_quote = squote_args[si-1];
            if (i != start_quote)
            {
                int size = i - start_quote;
                int j;
                for (j = start_quote; j <= i; ++j)
                {
                    size += strlen(argv[j]);
                }
                char *tmp = (char*)malloc(size*sizeof(char));
                strcpy(tmp, argv[start_quote]);
                free(argv[start_quote]);
                argv[start_quote] = tmp;
                for (j = si+1; j <= i; ++j)
                {
                    strcat(argv[start_quote], " ");
                    strcat(argv[start_quote], argv[j]);
                    argv[j] = " ";
                }
                squote_args[--si] = -1;
            }
        }
        argv[i] = strdup(argv[i]);
    }
}

void handle_redirect(char *argv[512])
{
    int use_stdin_next = 0;
    int use_stdout_next = 0;
    int use_stderr_next = 0;

    int append = 0;
    int outerr = 0;
    int i;
    for (i = 0; argv[i]; ++i)
    {
        char *p = argv[i];

        if (use_stdout_next || use_stderr_next)
        {
            int fd = open(p, O_WRONLY|O_CREAT|append, 0644);
            if (use_stdout_next)
                dup2(fd, STDOUT_FILENO);
            if (use_stderr_next)
                dup2(fd, STDERR_FILENO);
            close(fd);
            argv[i] = " ";
            use_stdout_next = 0;
            use_stderr_next = 0;
            append = 0;
            continue;
        }
        if (use_stdin_next)
        {
            int fd = open(p, O_RDONLY);
            dup2(fd, STDIN_FILENO);
            close(fd);
            argv[i] = " ";
            continue;
        }

        //Redirect stdout
        if (p[0] == '>' ||
           (strlen(p) > 1 && p[1] == '>'))
        {
            int src = -1;
            append = 0;
            outerr = 0;
            if (p[0] == '>')
            {
                src = STDOUT_FILENO;
                p++;
            }
            else
            {
                if (p[0] == '&')
                    outerr = 1;
                else
                    src = p[0] - '0';
                if (src > 2)
                {
                    perror("dsh: Bad file descriptor");
                    exit(1);
                }
                p++;
                p++;
            }
            if (p[0] == '>')
            {
                append = O_APPEND;
                p++;
            }
            if (!p[0]) //NOT for >&1 >&2, only files
            {
                if (outerr)
                {
                    use_stdout_next = 1;
                    use_stderr_next = 1;
                }
                if (src == STDOUT_FILENO)
                    use_stdout_next = 1;
                if (src == STDERR_FILENO)
                    use_stderr_next = 1;
                argv[i] = " ";
                outerr = 0;
                continue;
            }
            int fd;
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
                if (outerr)
                {
                    dup2(fd, STDOUT_FILENO);
                    dup2(fd, STDERR_FILENO);
                } else
                {
                    dup2(fd, src);
                }
                close(fd);
            } else //redirect to file
            {
                fd = open(p, O_WRONLY|O_CREAT|append, 0644);
                if (outerr)
                {
                    dup2(fd, STDOUT_FILENO);
                    dup2(fd, STDERR_FILENO);
                } else
                {
                    dup2(fd, src);
                }
                close(fd);
            }
            argv[i] = " ";
            append = 0;
            outerr = 0;
            continue;
        }
        if (p[0] == '<' ||
           ((strlen(p) > 1 && p[1] == '<')))
        {
            if (p[0] != '<' && !(p[0] == '0' && p[1] == '<'))
            {
                perror("dsh: Bad file descriptor");
                exit(1);
            }
            int src = STDIN_FILENO;
            if (p[0] != '<')
                p++;
            p++;
            if (!p[0])
            {
                use_stdin_next = 1;
                argv[i] = " ";
                continue;
            }
            int fd = open(p, O_RDONLY);
            dup2(fd, src);
            close(fd);
            argv[i] = " ";
            continue;
        }
    }
}

void handle_pipes(char *cmd, int num_pipes)
{
    char *pipe_cmds[512] = {};
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
    char *argv_buf[512] = {};
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
                argv_buf[j] = strsep(&pipe_cmds[i], " ");

            char **argv = expand(argv_buf);
            combine_quoted(argv);
            handle_redirect(argv);
            remove_spaces(argv);
            execvp(argv[0], argv);
            printf("dsh: Command not found: %s\n", argv[0]);
            exit(127);
        }
        int exit_code;
        waitpid(pid, &exit_code, 0);

        if (i != 0)
            close(pfds[(i-1)*2]);
        if (i != num_pipes)
            close(pfds[i*2+1]);

        if (WIFEXITED(exit_code))
        {
            set_exit_code(WEXITSTATUS(exit_code));
        }
        pid = -1;
    }
}

void run(char *input)
{
    char *cmds[512] = {};
    char *argv_buf[512] = {};
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
            argv_buf[j] = strsep(&cmds[i], " ");
        }
        argv_buf[j] = 0;

        if (strlen(*argv_buf) == 0)
        {
            continue;
        }

        if (handle_builtins(argv_buf))
            continue;

        pid = fork();
        if (pid == 0)
        {
            char **argv = expand(argv_buf);
            combine_quoted(argv);
            handle_redirect(argv);
            remove_spaces(argv);
            execvp(argv[0], argv);
            printf("dsh: Command not found: %s\n", argv[0]);
            exit(127);
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

void signal_process(int signo)
{
    if (pid > 0)
    {
        kill(pid, signo);
    }
}

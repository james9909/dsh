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
#include "aliases.h"

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

char **split(char *cmd) {
    char **ret, **ptr;
    int len = strlen(cmd);
    int i;
    ret = ptr = (char **) calloc(len, sizeof(char *));
    if (!(*cmd)) {
        *ptr = strdup("");
    }

    char quoted = 0;
    int length;
    for (i = 0; i < len; i++) {
        if (cmd[i] == '\'' || cmd[i] == '"') {
            quoted = cmd[i];
            length = 1;
            while (cmd[i+length] && cmd[i+length] != quoted) {
                length++;
            }
            *ptr = (char *) calloc(length, sizeof(char));
            strncpy(*ptr++, cmd+i+1, length-1);
            i += length;
            quoted = 0;
        } else if (cmd[i] == ' ') {
            if (ptr != NULL) {
                ptr++;
            }
        } else {
            length = 1;
            while (cmd[i+length] && cmd[i+length] != '\'' && cmd[i+length] != '"' && cmd[i+length] != ' ') {
                length++;
            }
            *ptr = (char *) calloc(length, sizeof(char));
            strncpy(*ptr++, cmd+i, length);
            i += length;
        }
    }
    return ret;
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

            char **argv = split(pipe_cmds[i]);

            argv = expand(argv);
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

        char **argv = split(cmd);

        if (strlen(*argv) == 0)
        {
            free(argv[0]);
            free(argv);
            continue;
        }

        if (handle_builtins(argv))
            continue;

        pid = fork();
        if (pid == 0)
        {
            argv = expand(argv);
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

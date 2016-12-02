#pragma once

typedef struct _Command
{
    char *argv[512];
    int argc;

    char stdin_redir_f[512];

    int stdout_redir;
    char stdout_redir_f[512];
    int stdout_append;

    int stderr_redir;
    char stderr_redir_f[512];
    int stderr_append;

    int dont_wait;

    int pipe_in;
    int pipe_out;

    struct _Command *pipe_to;
    struct _Command *piped_from;

    struct _Command *and_to;
    struct _Command *and_from;

    struct _Command *or_to;
    struct _Command *or_from;

    struct _Command *next_cmd;
    struct _Command *prev_cmd;
} Command;

void clear_cmd(Command *c);
void handle_redirects(Command *c);
void handle_pipes(Command *c);
void expand(Command *c);
Command *next_cmd(Command *c);
void free_cmds(Command *c);
void print_cmds(Command *c);

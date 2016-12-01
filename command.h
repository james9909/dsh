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

    struct _Command *pipe_to;
    struct _Command *piped_from;

    struct _Command *and_to;
    struct _Command *and_from;

    struct _Command *or_to;
    struct _Command *or_from;

    struct _Command *next_cmd;
    struct _Command *prev_cmd;
} Command;

void handle_redirects(Command *c);
Command *get_piped(Command *c);
Command *get_and(Command *c);
Command *get_or(Command *c);
Command *get_next(Command *c);
Command *next_cmd(Command *c);

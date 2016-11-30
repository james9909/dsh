#pragma once

struct Command
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

    struct Command *pipe_to;
    struct Command *piped_from;

    struct Command *and_to;
    struct Command *and_from;

    struct Command *or_to;
    struct Command *or_from;

    struct Command *next_cmd;
    struct Command *prev_cmd;
};

typedef struct Command Command;

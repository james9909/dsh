#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include "command.h"

int parse_cmdlist();

char *p;
char *p_end;

int accept(char c)
{
    if (p[0] == c)
    {
        p++;
        return 1;
    }
    return 0;
}

int accepts(char *c)
{
    int ret = 1;
    int i;
    for (i = 0; i < strlen(c); ++i)
    {
        if (p[i] != c[i])
            return 0;
    }
    p += strlen(c);
    return 1;
}

void ignore_whitespace()
{
    while (accept(' ') || accept('\t') || accept('\r'));
}

void expect(char c)
{
    if (p[0] == c)
    {
        p++;
        return;
    }
    fprintf(stderr, "Parse error: Expected %c, got %c\n", c, p[0]);
    exit(1);
}

//Allocs mem, assigns to *dst
int parse_word(char **dst)
{
#ifdef DEBUG
    fprintf(stderr, "In parse_word(). p: %s\n", p);
    fprintf(stderr, "p: %p, end: %p\n", p, p_end);
#endif
    char t[512];
    int i = 0;

    if (p[0] == 0)
        return 0;

    ignore_whitespace();

    if (p[0] == '"' || p[0] == '\'')
    {
        int quotes = 1;
        int other_quotes = 0;
        char quote = p[0];
        char other_quote = quote == '"' ? '\'' : '"';
        p++;

        for (i = 0;;++i)
        {
            if (quotes == other_quotes)
            {
                if (p[i] == quote)
                {
                    quotes++;
                }
                else if (p[i] == other_quote)
                {
                    other_quotes--;
                }
            }
            else
            {
                if (p[i] == other_quote)
                {
                    other_quotes++;
                }
                else if (p[i] == quote)
                {
                    quotes--;
                }
            }
            if (quotes == 0)
                break;
            if (p[i] == '\0')
            {
                fprintf(stderr, "Parsing error near %c at pos %d\n", p[i-1], i-1);
                fprintf(stderr, "Maybe your quotes?\n");
                exit(1);
            }
            t[i] = p[i];
        }
        t[i] = 0;
        *dst = strdup(t);
        p += i+1;
        return 1;
    }
    else
    {
        if (p[0] == '\0'
         || p[0] == ' '
         || p[0] == ';'
         || p[0] == '|'
         || p[0] == '&'
         || p[0] == '>'
         || p[0] == '<')
            return 0;
        if (p[0] == '=')
        {
            t[0] = '=';
            t[1] = 0;
            *dst = strdup(t);
            p += 1;
            return 1;
        }

        for (i = 0;;++i)
        {
            if (p[i] == '\0'
             || p[i] == ' '
             || p[i] == ';'
             || p[i] == '|'
             || p[i] == '&'
             || p[i] == '>'
             || p[i] == '<'
             || p[i] == '=')
                break;

            t[i] = p[i];
        }
        t[i] = 0;
        *dst = strdup(t);
        p += i;
        return 1;
    }

    return 0;
}

int parse_redirect(Command *a)
{
#ifdef DEBUG
    fprintf(stderr, "In parse_redirect()\n");
#endif
    // &>
    if (p[0] == '&' && p[1] == '>')
    {
        p += 2;
        if (accept('>'))
        {
            a->stdout_append = 1;
            a->stderr_append = 1;
        }
        ignore_whitespace();

        char *f;
        parse_word(&f);
        strcpy(a->stdout_redir_f, f);
        strcpy(a->stderr_redir_f, f);
        free(f);
        return 1;
    }
    // fd>
    if (1 <= (p[0] - '0') && (p[0] - '0') <= 2 && p[1] == '>')
    {
        int fd = p[0] - '0';
        p += 2;
        if (accept('>'))
        {
            if (fd == STDOUT_FILENO)
                a->stdout_append = 1;
            if (fd == STDERR_FILENO)
                a->stderr_append = 1;
        }
        ignore_whitespace();

        if (accept('&'))
        {
            int new_fd = p[0] - '0';
            if (new_fd < 1 || new_fd > 2)
            {
                fprintf(stderr, "Invalid file descriptor.\n");
                exit(1);
            }
            if (fd == STDOUT_FILENO)
                a->stdout_redir = new_fd;
            if (fd == STDERR_FILENO)
                a->stderr_redir = new_fd;
            return 1;
        }
        char *f;
        parse_word(&f);
        if (fd == STDOUT_FILENO)
            strcpy(a->stdout_redir_f, f);
        if (fd == STDERR_FILENO)
            strcpy(a->stderr_redir_f, f);
        free(f);
        return 1;
    }
    // >
    if (p[0] == '>')
    {
        p++;
        if (accept('>'))
        {
            a->stdout_append = 1;
        }
        ignore_whitespace();

        if (accept('&'))
        {
            int new_fd = p[0] - '0';
            if (new_fd < 1 || new_fd > 2)
            {
                fprintf(stderr, "Invalid file descriptor.\n");
                exit(1);
            }
            a->stdout_redir = new_fd;
            return 1;
        }
        int fd = STDOUT_FILENO;
        char *f;
        parse_word(&f);
        strcpy(a->stdout_redir_f, f);
        free(f);
        return 1;
    }
    // <
    if (p[0] == '<')
    {
        p++;
        ignore_whitespace();

        char *f;
        parse_word(&f);
        strcpy(a->stdin_redir_f, f);
        free(f);
        return 1;
    }

    return 0;
}

int parse_expr(Command *a)
{
#ifdef DEBUG
    fprintf(stderr, "In parse_expr()\n");
#endif
    char *s;
    if (parse_redirect(a))
    {
        return 1;
    }
    else if (parse_word(&a->argv[a->argc]))
    {
        a->argc++;
        return 1;
    }
    return 0;
}

int parse_exprs(Command *a)
{
#ifdef DEBUG
    fprintf(stderr, "In parse_exprs()\n");
#endif
    if (parse_expr(a))
    {
        ignore_whitespace();
        while (parse_expr(a))
        {
            ignore_whitespace();
        }
        return 1;
    }
    return 0;
}

int parse_cmd(Command *a)
{
#ifdef DEBUG
    fprintf(stderr, "In parse_cmd()\n");
#endif
    if (parse_exprs(a))
    {
        return 1;
    }
    return 0;
}

int parse_pipe(Command **a)
{
#ifdef DEBUG
    fprintf(stderr, "In parse_pipe()\n");
#endif
    if (parse_cmd(*a))
    {
        ignore_whitespace();
        while (p[1] != '|' && accept('|'))
        {
            ignore_whitespace();
            Command *b = (Command*)calloc(1, sizeof(Command));
            if (parse_cmd(b))
            {
                (*a)->pipe_to = b;
                b->piped_from = *a;
            }
            *a = b;
        }
        return 1;
    }
    return 0;
}

int parse_connector(Command *a)
{
#ifdef DEBUG
    fprintf(stderr, "In parse_connector()\n");
#endif
    if (parse_pipe(&a))
    {
        ignore_whitespace();
        while (accepts("&&"))
        {
            ignore_whitespace();
            Command *b = (Command*)calloc(1, sizeof(Command));
            if (parse_connector(b))
            {
                a->and_to = b;
                b->and_from = a;
            }
            a = b;
        }
        while (accepts("||"))
        {
            ignore_whitespace();
            Command *b = (Command*)calloc(1, sizeof(Command));
            if (parse_connector(b))
            {
                a->or_to = b;
                b->or_from = a;
            }
            a = b;
        }
        return 1;
    }
    return 0;
}

int parse_cmdlist(Command *a)
{
#ifdef DEBUG
    fprintf(stderr, "In parse_cmdlist()\n");
#endif
    if (parse_connector(a))
    {
        ignore_whitespace();
        while (accept(';'))
        {
            ignore_whitespace();
            Command *b = (Command*)calloc(1, sizeof(Command));
            if (parse_connector(b))
            {
                a->next_cmd = b;
                b->prev_cmd = a;
            }
            a = b;
        }
        return 1;
    }
    return 0;
}

void free_cmds(Command *c)
{
    if (c == NULL)
        return;
    free(c);
    free_cmds(c->pipe_to);
    free_cmds(c->and_to);
    free_cmds(c->or_to);
    free_cmds(c->next_cmd);
}

#ifdef DEBUG
void print_cmd(Command *c)
{
    printf("Command at %p\n", c);
    int i;
    for (i = 0; i < c->argc; ++i)
    {
        printf("argv[%d]: %s\n", i, c->argv[i]);
    }
    printf("argc: %d\n", c->argc);
    printf("stdin_redir_f: %s\n"
           "stdout_redir: %d\n"
           "stderr_redir: %d\n"
           "stdout_redir_f: %s\n"
           "stderr_redir_f: %s\n"
           "stdout_append: %d\n"
           "stderr_append: %d\n",
           c->stdin_redir_f, c->stdout_redir, c->stderr_redir,
           c->stdout_redir_f, c->stderr_redir_f,
           c->stdout_append, c->stderr_append);
    printf("Pipes to: %p\n", c->pipe_to);
    printf("Piped from: %p\n", c->piped_from);
    printf("And to: %p\n", c->and_to);
    printf("And from: %p\n", c->and_from);
    printf("Or to: %p\n", c->or_to);
    printf("Or from: %p\n", c->or_from);
    printf("Next cmd: %p\n", c->next_cmd);
    printf("Prev cmd: %p\n", c->prev_cmd);
    printf("-----------------------------\n");
}

void print_cmds(Command *c)
{
    if (c == NULL)
        return;
    print_cmd(c);
    print_cmds(c->pipe_to);
    print_cmds(c->and_to);
    print_cmds(c->or_to);
    print_cmds(c->next_cmd);
}
#endif

Command *parse(char *input)
{
#ifdef DEBUG
    printf("Parser input: %s\n", input);
#endif
    p = input;
    p_end = p + strlen(p);

    Command *a = (Command*)calloc(1, sizeof(Command));
    a->argc = 0;
    a->stdout_append = 0;
    a->stderr_append = 0;
    a->stdout_redir = 0;
    a->stderr_redir = 0;
    memset(a->stdin_redir_f, 0, sizeof(a->stdin_redir_f));
    memset(a->stdout_redir_f, 0, sizeof(a->stdout_redir_f));
    memset(a->stderr_redir_f, 0, sizeof(a->stderr_redir_f));
    a->pipe_in = 0;
    a->pipe_out = 0;
    a->pipe_to = 0;
    a->piped_from = 0;
    a->and_to = 0;
    a->and_from = 0;
    a->or_to = 0;
    a->or_from = 0;
    a->next_cmd = 0;
    a->prev_cmd = 0;
    parse_cmdlist(a);

    return a;

#ifdef DEBUG
    print_cmds(a);
#endif
}

#ifdef PARSER_ALONE
int main()
{
    char s[512];
    fgets(s, 512, stdin);
    s[strcspn(s, "\n")] = 0;
    parse(s);
    return 0;
}
#endif

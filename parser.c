#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include "command.h"

int parse_word(char **dst);
int parse_cmdlist();
int parse_statement(Command *c);
Command *parse(char *input);

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
    fprintf(stderr, "dsh: Parse error: Expected %c, got %c\n", c, p[0]);
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
                fprintf(stderr, "dsh: Parsing error near %c at pos %d\n", p[i-1], i);
                return 0;
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
    if (p[0] == '&' && (p >= p_end || p[1] == '>'))
    {
        p += 2;
        if (accept('>'))
        {
            a->stdout_append = 1;
            a->stderr_append = 1;
        }
        ignore_whitespace();

        if (p >= p_end)
        {
            fprintf(stderr, "dsh: No redirect target.\n");
            a->abort = 1;
            return 1;
        }
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
            p++;
            if (new_fd < 1 || new_fd > 2)
            {
                fprintf(stderr, "dsh: Invalid file descriptor.\n");
                return 1;
            }
            if (fd == new_fd)
            {
                fprintf(stderr, "dsh: Redirecting to same fd.\n");
                return 1;
            }
            if (fd == STDOUT_FILENO)
                a->stdout_redir = new_fd;
            if (fd == STDERR_FILENO)
                a->stderr_redir = new_fd;
            return 1;
        }
        if (p >= p_end)
        {
            fprintf(stderr, "dsh: No redirect target.\n");
            a->abort = 1;
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
    if (accept('>'))
    {
        if (accept('>'))
        {
            a->stdout_append = 1;
        }
        ignore_whitespace();

        if (accept('&'))
        {
            int new_fd = p[0] - '0';
            p++;
            if (new_fd < 1 || new_fd > 2)
            {
                fprintf(stderr, "dsh: Invalid file descriptor.\n");
                return 1;
            }
            if (new_fd == STDOUT_FILENO)
            {
                fprintf(stderr, "dsh: Redirecting to same fd detected.\n");
                return 1;
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
    if (accept('<'))
    {
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
        if (*p == 0)
            return 0;

        while (p < p_end && p[1] != '|' && accept('|'))
        {
            ignore_whitespace();
            Command *b = (Command*)calloc(1, sizeof(Command));
            parse_cmd(b);
            (*a)->pipe_to = b;
            b->piped_from = *a;
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
            parse_connector(b);
            a->and_to = b;
            b->and_from = a;
            a = b;
        }
        while (accepts("||"))
        {
            ignore_whitespace();
            Command *b = (Command*)calloc(1, sizeof(Command));
            parse_connector(b);
            a->or_to = b;
            b->or_from = a;
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
        if (p[0] == '&' && p[1] != '&')
        {
            p++;
            ignore_whitespace();
            a->dont_wait = 1;
        }
        while (accept(';'))
        {
            ignore_whitespace();
            Command *b = (Command*)calloc(1, sizeof(Command));
            parse_connector(b);
            a->next_cmd = b;
            b->prev_cmd = a;
           a = b;
        }
        return 1;
    }
    return 0;
}

int parse_if(Command *c)
{
    if (!accepts("if"))
        return 0;
    ignore_whitespace();
    c->argv[0] = strdup("if");

    char t[512] = {};
    int i = 0;
    while (!accepts("then"))
    {
        if (p >= p_end)
        {
            fprintf(stderr, "dsh: No corresponding `then` for `if` block\n");
            c->abort = 1;
            return 1;
        }
        t[i++] = p[0];
        p++;
    }
    c->condition = (Command*)malloc(sizeof(Command));
    clear_cmd(c->condition);

    char *bkp = p;
    char *bkp_end = p_end;
    p = t;
    p_end = strchr(t, '\0');
    parse_statement(c->condition);
    p = bkp;
    p_end = bkp_end;
    memset(t, 0, sizeof(t));

    i = 0;
    while (!accepts("fi"))
    {
        if (p >= p_end)
        {
            fprintf(stderr, "dsh: `if` never closed.\n");
            c->abort = 1;
            return 1;
        }
        t[i++] = p[0];
        p++;
    }
    c->cond_cmd = (Command*)malloc(sizeof(Command));
    clear_cmd(c->cond_cmd);

    bkp = p;
    bkp_end = p_end;
    p = t;
    p_end = strchr(t, '\0');
    parse_statement(c->cond_cmd);
    p = bkp;
    p_end = bkp_end;
    return 1;
}


int parse_statement(Command *c)
{
    if (parse_if(c))
    {
        return 1;
    }
    if (parse_cmdlist(c))
    {
        return 1;
    }
    return 0;
}

Command *parse(char *input)
{
#ifdef DEBUG
    printf("Parser input: %s\n", input);
#endif
    p = input;
    p_end = p + strlen(p);

    Command *a = (Command*)calloc(1, sizeof(Command));
    clear_cmd(a);
    parse_statement(a);

#ifdef DEBUG
    print_cmds(a);
#endif

    return a;
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

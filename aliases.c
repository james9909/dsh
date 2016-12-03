#include <string.h>
#include <stdlib.h>
#include <stdio.h>

#include "parser.h"
#include "aliases.h"
#include "executor.h"

char aliases[2][512][512] = {};
int max = 0;

void print_aliases()
{
    int i;
    for (i = 0; i < max; ++i)
    {
        printf("%s=\"%s\"\n", aliases[0][i], aliases[1][i]);
    }
}

int find_alias(char *name)
{
    int i;
    for (i = 0; i < max; ++i)
    {
        if (strcmp(name, aliases[0][i]) == 0)
            return i;
    }
    return -1;
}

void add_alias(char *alias, char *replacement)
{
    int index = -1;
    if ((index = find_alias(alias)) == -1)
    {
        strcpy(aliases[0][max], alias);
        strcpy(aliases[1][max], replacement);
        max++;
    }
    else
    {
        strcpy(aliases[1][index], replacement);
    }
}

void handle_aliases(Command *c)
{
    int i;
    for (i = 0; i < max; ++i)
    {
        if (strcmp(c->argv[0], aliases[0][i]) == 0)
        {
            Command *a = parse(aliases[1][i]);
            int i;
            for (i = 1; i < a->argc; ++i)
            {
                c->argv[i+a->argc-1] = c->argv[i];
            }
            free(c->argv[0]);
            for (i = 0; i < a->argc; ++i)
            {
                c->argv[i] = strdup(a->argv[i]);
            }
            c->argc += a->argc - 1;
            if (a->pipe_to)
            {
                c->pipe_to = a->pipe_to;
                a->pipe_to = 0;
            }
            if (a->and_to)
            {
                c->and_to = a->and_to;
                a->and_to = 0;
            }
            if (a->or_to)
            {
                c->or_to = a->or_to;
                a->or_to = 0;
            }
            if (a->next_cmd)
            {
                c->next_cmd = a->next_cmd;
                a->next_cmd = 0;
            }
            free_cmds(a);
        }
    }
}

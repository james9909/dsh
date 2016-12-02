#include <string.h>
#include <stdlib.h>
#include <stdio.h>

#include "parser.h"
#include "aliases.h"
#include "executor.h"

char aliases[2][512][512] = {};
int max = 0;

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
            int j;
            for (j = 1; j < c->argc; ++j)
            {
                c->argv[j+a->argc-1] = c->argv[j];
            }
            /* c->argv[j+a->argc-1] = 0; */
            c->argc += a->argc - 1;
            for (j = 0; j < a->argc; ++j)
            {
                c->argv[j] = strdup(a->argv[j]);
            }
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

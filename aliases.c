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
            for (j = 1; j < a->argc; ++j)
            {
                c->argv[j+a->argc-1] = c->argv[j];
                c->argc++;
            }
            for (j = 0; j < a->argc; ++j)
            {
                c->argv[j] = a->argv[j];
            }
            free_cmds(a);
        }
    }
}

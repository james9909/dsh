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

void add_alias(char *alias, char* replacement)
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

/*
void handle_aliases(char *argv[512])
{
    int i;
    for (i = 0; argv[i]; ++i)
    {
        int index = 0;
        if ((index = find_alias(argv[i])) != -1)
        {
            free(argv[i]);
            argv[i] = split(aliases[1][index]);
        }
    }
}
*/

void handle_aliases(Command *c)
{
    int i;
    for (i = 0; i < max; ++i) {
        if (strcmp(c->argv[0], aliases[0][i]) == 0) {
            char *cmd = strdup(aliases[1][i]);
            Command *a = parse(cmd);
            int i;
            for (i = 1; i < a->argc; ++i) {
                c->argv[i+a->argc-1] = c->argv[i];
            }
            for (i = 0; i < a->argc; ++i) {
                c->argv[i] = a->argv[i];
            }
            free_cmds(a);
        }
    }
}

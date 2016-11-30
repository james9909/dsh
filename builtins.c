#include <glob.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <wordexp.h>
#include <assert.h>

#include "command.h"
#include "executor.h"
#include "builtins.h"
#include "aliases.h"

void single_expand(char **arg)
{
    glob_t globbuf;
    glob(*arg, GLOB_TILDE|GLOB_NOCHECK, NULL, &globbuf);
    *arg = strdup(globbuf.gl_pathv[0]);
    globfree(&globbuf);
}

int is_builtin(char *cmd)
{
    if (strcmp(cmd, "cd") == 0
     || strcmp(cmd, "exit") == 0
     || strcmp(cmd, "alias") == 0)
        return 1;
    if (strchr(cmd, '='))
        return 1;
    return 0;
}

void handle_builtins(Command *c)
{
    if (strcmp(c->argv[0], "exit") == 0)
    {
        int exit_code = 0;
        if (c->argc > 1)
        {
            exit_code = atoi(c->argv[1]);
        }
        exit(exit_code);
    }
    if (strcmp(c->argv[0], "cd") == 0)
    {
        fprintf(stderr, "cd: Not yet implemented\n");
        exit(1);
    }
    if (strcmp(c->argv[0], "alias") == 0)
    {
        int i;
        for (i = 0; i < c->argc; ++i)
        {
            printf("argv[%d]: %s\n", i, c->argv[i]);
        }
        if (c->argc < 2)
        {
            fprintf(stderr, "alias: Syntax error.\n");
            exit(1);
        }
        char *o, *r;
        o = c->argv[1];
        r = strchr(o, '=');
        r[0] = 0;
        r++;
        printf("alias \"%s\" to \"%s\"\n", o, r);
        printf("Not yet implemented\n");
        return;
    }
    if (strchr(c->argv[0], '='))
    {
        char *o, *r;
        o = c->argv[0];
        r = strchr(o, '=');
        r[0] = 0;
        r++;
        printf("environmental variable \"%s\" to \"%s\"\n", o, r);
        printf("Not yet implemented\n");
        return;
    }
    assert(0); //if triggered, is_builtin is not right
}

/*
int handle_builtins(char **argl)
{
    if (strcmp(argl[0], "exit") == 0)
    {
        exit(0);
        return 1;
    }
    if (strcmp(argl[0], "cd") == 0)
    {
        char *path;

        int argc;
        for (argc = 0; argl[argc + 1]; argc++);

        if (argc == 0)
        {
            path = "~";
        } else if (argc == 1)
        {
            path = argl[1];
        } else
        {
            printf("cd: Too many arguments\n");
            return 1;
        }
        single_expand(&path);

        int status = chdir(path);
        if (status == -1)
        {
            printf("cd: %s: %s\n", strerror(errno), path);
        }
        free(path);
        return 1;
    }
    if (strcmp(argl[0], "alias") == 0)
    {
        char *alias = argl[1];
        char *replacement;
        char *tmp;
        int index = -1;

        int i;
        if ((tmp = strchr(alias, '=')))
        {
            index = tmp - alias;
            alias[index] = 0;
            tmp++; //expect '\0' (prev '=')

            char quote_used;
            if (tmp[0] == '"' || tmp[0] == '\'')
            {
                quote_used = tmp[0];
                tmp++;
                int length = strlen(tmp);
                if (tmp[length-1] == quote_used)
                {
                    tmp[length-1] = 0;
                }
                else
                {
                    i = 2;
                    int ln;
                    while (1)
                    {
                        if (!argl[i])
                        {
                            fprintf(stderr, "alias: Invalid syntax.\n");
                            exit(1);
                        }

                        ln = strlen(argl[i]);
                        length += 1 + ln;
                        if (argl[i][ln-1] == quote_used)
                        {
                            argl[i][ln-1] = 0;
                            break;
                        }
                        i++;
                    }
                }
                length++;
                replacement = (char*)calloc(length, sizeof(char));
                strcpy(replacement, tmp);
                for (i = 2; argl[i]; ++i)
                {
                    strcat(replacement, " ");
                    strcat(replacement, argl[i]);
                }
            }
            else
            {
                fprintf(stderr, "alias: Invalid syntax. Did you forget quotes?\n");
                exit(1);
            }
        } else
        {
            fprintf(stderr, "alias: Invalid syntax.\n");
            exit(1);
        }

        add_alias(alias, replacement);
        free(replacement);
        return 1;
    }
    if (strchr(argl[0], '='))
    {
        char *env = argl[0];
        int len_name = 0;
        while (env[len_name] != '=') len_name++;
        int len_value = strlen(env) - len_name;

        char first, last;
        first = env[len_name+1];
        last = env[len_name + len_value - 1];
        if (first != '\'' && first != '"' && last != first) {
            fprintf(stderr, "alias: Invalid syntax. Did you forget quotes?\n");
            exit(1);
        }

        char *name = (char *) calloc(len_name, sizeof(char));
        char *value = (char *) calloc(len_value, sizeof(char));
        strncpy(name, env, len_name);
        strncpy(value, env+len_name+2, len_value-3);

        setenv(name, value, 1);
        free(name);
        free(value);
        return 1;
    }
    return 0;
}
*/

char **expand(char **argl) {
    glob_t globbuf;
    int i, j;
    int flags = GLOB_TILDE | GLOB_NOCHECK;

    int size = 128;
    char **new = (char **) calloc(size, sizeof(char*));
    int newi = 0;

    for (i = 0; argl[i] != NULL; i++)
    {
        glob(argl[i], flags , NULL, &globbuf);
        for (j = 0; j < globbuf.gl_pathc; ++j)
        {
            new[newi++] = strdup(globbuf.gl_pathv[j]);
            if (newi > size)
            {
                new = realloc(new, size*2);
            }
        }
    }
    globfree(&globbuf);
    return new;
}

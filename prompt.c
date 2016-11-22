#include <pwd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <time.h>
#include <unistd.h>

#include "colors.h"
#include "prompt.h"

struct passwd *p;
int exit_code;

void set_exit_code(int n) {
    exit_code = n;
}

void load_prompt() {
    PROMPT = "{GREEN}[{time}] {RED}{username}{RESET}@{MAGENTA}{host} {BLUE}[{pwd}] {GREEN}{git_info} {sign}{RESET} "; // TODO: read from file?
    int i, lb, rb;
    for (i = lb = rb = 0; i < strlen(PROMPT); i++) {
        if (PROMPT[i] == '{') {
            lb++;
        } else if (PROMPT[i] == '}') {
            rb++;
        }
    }
    if (lb != rb) {
        printf("Invalid prompt\n");
        PROMPT = "$ ";
    }
}

void git_info() {
    FILE *fp;
    char output[1024];

    fp = popen("git status 2> /dev/null", "r");
    if (fp) {
        fgets(output, sizeof(output), fp);
        if (strlen(output) == 0) {
            return;
        }
    }

    fp = popen("git branch 2> /dev/null | grep -Po \"\\* \\K(.+)\"", "r");
    if (fp) {
        fgets(output, sizeof(output), fp);
        output[strlen(output)-1] = 0;
        printf("(%s ", output);
        output[0] = 0;
    }

    fp = popen("git status -s 2> /dev/null", "r");
    if (fp) {
        fgets(output, sizeof(output), fp);
        if (strlen(output) == 0) {
            printf("✔");
        } else {
            printf("✘");
        }
        printf(")");
    }
}

void print_prompt() {
    int i, length;
    p = getpwuid(getuid());
    for (i = 0; i < strlen(PROMPT); i++) {
        if (PROMPT[i] == '{') {
            length = 1;
            while (PROMPT[i+length] != '}') {
                length++;
            }
            char *var = (char *)calloc(length, sizeof(char));
            strncpy(var, PROMPT+i+1, length-1);
            print_variable(var);
            free(var);

            i += length;
        } else {
            printf("%c", PROMPT[i]);
        }
    }
}

void print_variable(char *var) {
    time_t raw;
    time(&raw);
    struct tm *timeinfo = localtime(&raw);
    if (strcmp(var, "time") == 0) {
        printf("%02d:%02d",
            timeinfo->tm_hour % 12,
            timeinfo->tm_min
            );
    } else if (strcmp(var, "username") == 0) {
        if (!p) {
            printf("unknown");
        } else {
            printf("%s", p->pw_name);
        }
    } else if (strcmp(var, "host") == 0) {
        char host[256];
        gethostname(host, sizeof(host));
        printf("%s", host);
    } else if (strcmp(var, "pwd") == 0) {
        char cwd[256];
        char *home = p->pw_dir;
        getcwd(cwd, sizeof(cwd));

        char *condensed = cwd;
        long len = strlen(home)-1;
        if (strncmp(cwd, home, len) == 0) {
            condensed[len] = '~';
            condensed += len;
        }
        printf("%s", condensed);
    } else if (strcmp(var, "sign") == 0) {
        if (exit_code == 0) {
            printf(GREEN "$" RESET);
        } else {
            printf(RED "$" RESET);
        }
    } else if (strcmp(var, "git_info") == 0) {
        git_info();
    } else if (strcmp(var, "BLUE") == 0) {
        printf(BLUE);
    } else if (strcmp(var, "GREEN") == 0) {
        printf(GREEN);
    } else if (strcmp(var, "MAGENTA") == 0) {
        printf(MAGENTA);
    } else if (strcmp(var, "RED") == 0) {
        printf(RED);
    } else if (strcmp(var, "RESET") == 0) {
        printf(RESET);
    } else if (strcmp(var, "YELLOW") == 0) {
        printf(YELLOW);
    }
}

#include <pwd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <time.h>
#include <unistd.h>

#include "colors.h"
#include "prompt.h"

int exit_code;

void set_exit_code(int n) {
    exit_code = n;
}

int get_exit_code() {
    return exit_code;
}

void load_prompt() {
    PROMPT = "{GREEN}[{time}] {RED}{username}{RESET}@{MAGENTA}{host} {BLUE}[{pwd}] {GREEN}{git_info} {sign}{RESET} "; // TODO: read from file?
    PROMPT = getenv("PROMPT");
    int i, lb, rb;
    for (i = lb = rb = 0; i < strlen(PROMPT); i++) {
        if (PROMPT[i] == '{' && (i == 0 || PROMPT[i-1] != '\\')) {
            lb++;
        } else if (PROMPT[i] == '}' && (i == 0 || PROMPT[i-1] != '\\')) {
            rb++;
        }
    }
    if (lb != rb) {
        printf("Invalid prompt\n");
        PROMPT = "$ ";
    }
}

char *git_info() {
    FILE *fp;
    char output[1024] = {};
    char *info = (char *) calloc(128, sizeof(char));

    fp = popen("git status 2> /dev/null", "r");
    if (fp) {
        fgets(output, sizeof(output), fp);
        pclose(fp);
        if (strlen(output) == 0) {
            return info;
        }
        output[0] = 0;
    }

    fp = popen("git branch 2> /dev/null | grep -Po \"\\* \\K(.+)\"", "r");
    if (fp) {
        fgets(output, sizeof(output), fp);
        pclose(fp);

        output[strlen(output)-1] = 0;
        sprintf(info, "(%s ", output);
        output[0] = 0;
    }

    fp = popen("git status -s 2> /dev/null", "r");
    if (fp) {
        fgets(output, sizeof(output), fp);
        pclose(fp);

        if (strlen(output) == 0) {
            strcat(info, "✔)");
        } else {
            strcat(info, "✘)");
        }
    }
    return info;
}

char *get_prompt() {
    load_prompt();
    int i, length;
    char *prompt = (char *) calloc(1024, sizeof(char));
    for (i = 0; i < strlen(PROMPT); i++) {
        if (PROMPT[i] == '{' && (i == 0 || PROMPT[i-1] != '\\')) {
            length = 1;
            while (PROMPT[i+length] != '}' && PROMPT[i+length-1] != '\\') {
                length++;
            }
            char *var = (char *)calloc(length, sizeof(char));
            strncpy(var, PROMPT+i+1, length-1);
            char *value = get_variable(var);
            strcat(prompt, value);
            free(var);
            free(value);

            i += length;
        } else if (PROMPT[i] != '\\') {
            strncat(prompt, &PROMPT[i], 1);
        }
    }
    return prompt;
}

char *get_variable(char *var) {
    char *value = (char *) calloc(256, sizeof(char));

    if (strcmp(var, "time") == 0) {
        time_t raw;
        time(&raw);
        struct tm *timeinfo = localtime(&raw);
        sprintf(value, "%02d:%02d",
            timeinfo->tm_hour % 12,
            timeinfo->tm_min
            );
    } else if (strcmp(var, "username") == 0) {
        strcpy(value, getenv("USER"));
    } else if (strcmp(var, "host") == 0) {
        gethostname(value, 256);
    } else if (strcmp(var, "pwd") == 0) {
        char cwd[256];
        char *home = getenv("HOME");
        getcwd(cwd, sizeof(cwd));

        long len = strlen(home);
        if (strncmp(cwd, home, len) == 0) {
            sprintf(value, "~%s", &cwd[len]);
        } else {
            strcpy(value, cwd);
        }
    } else if (strcmp(var, "sign") == 0) {
        if (exit_code == 0) {
            strcpy(value, GREEN "$" RESET);
        } else {
            strcpy(value, RED "$" RESET);
        }
    } else if (strcmp(var, "git_info") == 0) {
        free(value);
        value = git_info();
    } else if (strcmp(var, "BLUE") == 0) {
        strcpy(value, BLUE);
    } else if (strcmp(var, "GREEN") == 0) {
        strcpy(value, GREEN);
    } else if (strcmp(var, "MAGENTA") == 0) {
        strcpy(value, MAGENTA);
    } else if (strcmp(var, "RED") == 0) {
        strcpy(value, RED);
    } else if (strcmp(var, "RESET") == 0) {
        strcpy(value, RESET);
    } else if (strcmp(var, "YELLOW") == 0) {
        strcpy(value, YELLOW);
    }
    return value;
}

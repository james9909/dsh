#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>

#include "executor.h"
#include "builtins.h"

void run(char *input) {
    input[strlen(input)-1] = 0;
    if (!(*input)) {
        return;
    }

    int num_commands = 1;
    int i, j;
    for (i = 0; input[i]; i++) {
        if (input[i] == ';') num_commands++;
    }

    char *commands[num_commands];
    char *argv[512] = {};

    for (i = 0; i < num_commands; i++) {
        commands[i] = strsep(&input, ";");
        while (commands[i][0] == ' ') commands[i]++;

        for (j = 0; commands[i]; j++) {
            argv[j] = strsep(&commands[i], " ");
        }
        argv[j] = 0;

        if (handle_builtins(argv))
            continue;

        pid_t n = fork();
        if (n == 0) {
            execvp(argv[0], argv);
            printf("dsh: Command not found: %s\n", argv[0]);
        }
        int status;
        wait(&status);
    }
}

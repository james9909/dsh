#include <setjmp.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <readline/history.h>
#include <readline/readline.h>

#include "builtins.h"
#include "executor.h"
#include "prompt.h"
#include "aliases.h"

sigjmp_buf ctrlc;

static void handler(int signo) {
    if (signo == SIGINT) {
        printf("\n");
        siglongjmp(ctrlc, 1);
    }
    signal_process(signo);
}

int main()
{
    signal(SIGINT, handler);

    char *input;
    char *prompt;
    load_prompt();

    sigsetjmp(ctrlc, 1);

    add_alias("l", "ls");
    add_alias("ll", "ls -al");
    while (1) {
        prompt = get_prompt();
        input = readline(prompt);
        free(prompt);

        if (!input) { // EOF
            printf("\n");
            break;
        }

        if (input[0] != 0) add_history(input);

        run(input);
        free(input);
    }
    return 0;
}

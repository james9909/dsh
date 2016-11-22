#include <setjmp.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <readline/readline.h>

#include "builtins.h"
#include "executor.h"
#include "prompt.h"

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

    while (1) {
        prompt = get_prompt();
        input = readline(prompt);
        free(prompt);

        if (!input) break;

        run(input);
        free(input);
    }
    return 0;
}

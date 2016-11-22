#include <setjmp.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>

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

    char buf[512] = {};
    char *input = buf;
    load_prompt();

    sigsetjmp(ctrlc, 1);

    while (1) {
        print_prompt();

        // TODO: make fgets stop blocking SIGINT
        if (fgets(buf, sizeof(buf), stdin) == NULL) {
            exit(0);
        }
        run(input);
    }
    return 0;
}

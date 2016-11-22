#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>

#include "builtins.h"
#include "executor.h"
#include "prompt.h"

int main()
{
    char buf[512] = {};
    char *input = buf;
    load_prompt();
    while (1) {
        print_prompt();
        if (fgets(buf, sizeof(buf), stdin) == NULL) {
            break;
        }
        run(input);
    }
    return 0;
}

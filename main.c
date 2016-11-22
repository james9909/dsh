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
    while (1) {
        prompt();
        fgets(buf, sizeof(buf), stdin);
        run(input);
    }
    return 0;
}

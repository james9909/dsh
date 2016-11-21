#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>

#include "builtins.h"
#include "executor.h"

int main()
{
    char buf[512] = {};
    char *args = buf;
    while (1) {
        printf("%% ");
        fgets(buf, sizeof(buf), stdin);
        run(args);
    }
    return 0;
}

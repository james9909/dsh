#pragma once
#include "command.h"

void single_expand(char **arg);
int is_builtin(Command *c);
void handle_builtins(Command *c);
char **expand(char **argl);

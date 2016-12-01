#pragma once
#include "command.h"

void expand_path(char **arg);
int is_builtin(Command *c);
void handle_builtins(Command *c);

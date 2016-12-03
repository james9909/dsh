#pragma once
#include "command.h"

void print_aliases();
void add_alias(char *alias, char *replacement);
void handle_aliases(Command *c);

#pragma once
#include "command.h"

int get_pid();
int exec(Command *c);
void run(Command *c);
void signal_process(int signo);

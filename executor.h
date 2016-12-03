#pragma once
#include "command.h"

int get_pid();
void run(Command *c);
void signal_process(int signo);

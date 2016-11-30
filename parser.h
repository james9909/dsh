#pragma once

//Only parse() needs to be exposed
Command *parse(char *input);
void free_cmds(Command *c);

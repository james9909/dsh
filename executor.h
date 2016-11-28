#pragma once

void remove_space(char *argv[512]);
void handle_redirect(char *argv[512]);
char **split(char *cmd);
void handle_pipes(char *cmd, int num_pipes);
void run(char *input);
void signal_process(int signo);

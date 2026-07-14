#ifndef SHELL_H
#define SHELL_H

void read_command(char* buffer, int max_size);
void execute_command(char* command, char* args);
void shell_main(void);

#endif

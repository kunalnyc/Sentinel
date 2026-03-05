#ifndef SHELL_H
#define SHELL_H

#define SHELL_MAX_INPUT  128
#define SHELL_MAX_LINES  40

void shell_init(void);
void shell_handle_key(char c);

#endif
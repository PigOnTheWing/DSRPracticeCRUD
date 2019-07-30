#ifndef CLIENT_READLINE_SETTINGS_H
#define CLIENT_READLINE_SETTINGS_H

#include <stdio.h>
#include <string.h>
#include <readline/readline.h>

char **crud_completion(const char *text, int start, int end);
char *crud_generator(const char *text, int state);
int get_op_args(char *buf, char **container, int max_args);

#endif //CLIENT_READLINE_SETTINGS_H

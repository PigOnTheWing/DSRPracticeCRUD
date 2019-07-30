#include "../headers/readline_settings.h"

char *commands[] = {
        "create",
        "read",
        "update",
        "delete",
        "help",
        "exit",
        NULL
};

char **crud_completion(const char *text, int start, int end) {
    rl_attempted_completion_over = 1;
    return rl_completion_matches(text, crud_generator);
}

char *crud_generator(const char *text, int state) {
    static int list_index, len;
    char *command;

    if (!state) {
        list_index = 0;
        len = strlen(text);
    }

    while((command = commands[list_index++])) {
        if (!strncmp(command, text, len)) {
            return strdup(command);
        }
    }

    return NULL;
}

int get_op_args(char *buf, char **container, int max_args) {
    int cont_index = 0;
    char *arg = strtok(buf, " ");
    if (arg == NULL)
        return 0;

    do {
        container[cont_index++] = arg;
        arg = strtok(NULL, " ");
    }
    while (cont_index < max_args && arg != NULL);

    if (arg == NULL)
        return cont_index;
    else
        return -1;
}

#include <stdlib.h>
#include "headers/connection.h"
#include "headers/crud_functions.h"
#include "headers/readline_settings.h"

#define ARGS_MAX_LENGTH 10
#define BUF_LENGTH 100

int main(int argc, char** argv)
{
    int sfd, op_args_cnt;
    char *buffer, *json_string, response[BUF_LENGTH];
    char *op_args[ARGS_MAX_LENGTH];

    if (argc < 3) {
        printf("Usage: %s port host", argv[0]);
        exit(EXIT_FAILURE);
    }

    sfd = connect_to_server(argv[1], argv[2]);

    if (sfd == -1) {
        printf("Could not connect to server");
        exit(EXIT_FAILURE);
    }

    rl_attempted_completion_function = crud_completion;

    printf("Entered interactive mode\n");

    while (1) {
        buffer = readline(">> ");
        if (buffer) {

            op_args_cnt = get_op_args(buffer, op_args, ARGS_MAX_LENGTH);

            if (op_args_cnt == -1) {
                printf("Too many arguments\n");
                free(buffer);
                continue;
            }

            if (!strcmp(buffer, "exit")) {
                free(buffer);
                break;
            }

            if (!strcmp(buffer, "read")) {
                json_string = msg_read((const char**) op_args, op_args_cnt);
            }
            else if (!strcmp(buffer, "create")) {
                json_string = msg_create((const char**) op_args, op_args_cnt);
            }
            else if (!strcmp(buffer, "delete")) {
                json_string = msg_create((const char**) op_args, op_args_cnt);
            }
            else
                json_string = NULL;

            if (json_string == NULL) {
                printf("Wrong format\n");
                free(buffer);
                continue;
            }

            if (write(sfd, json_string, strlen(json_string)) == -1) {
                printf("Failed to send data to server");
            }

            if (read(sfd, &response, BUF_LENGTH) == -1) {
                printf("Failed to get a response");
            }

            printf("%s", response);
        }
    }

    exit(EXIT_SUCCESS);
}
#include <stdlib.h>
#include "headers/connection.h"
#include "headers/json_parsing.h"
#include "headers/readline_settings.h"

#define ARGS_MAX_LENGTH 10
#define BUF_LENGTH 1024

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
        printf("Could not connect to server\n");
        exit(EXIT_FAILURE);
    }

    rl_attempted_completion_function = crud_completion;

    printf("Entered interactive mode\nFor help, type help\n");

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
                write(sfd, "close", 10);
                free(buffer);
                close(sfd);
                break;
            }

            if (!strcmp(buffer, "help")) {
                printf("Command format: op_id msg_type args\nop_id: create/read/delete\n"
                       "msg_type: \n\tcoords, args - latitude longitude\n"
                       "\tperson args - first_name last_name age\n"
                       "Create:\n\tcreate msg_type device_id args\n"
                       "Read:\n\tread | read [ids]\n"
                       "Delete\n\tdelete [ids]\n");
                free(buffer);
                continue;
            }

            if (!strcmp(buffer, "read")) {
                json_string = msg_read((const char**) op_args + 1, op_args_cnt - 1);
            }
            else if (!strcmp(buffer, "create")) {
                json_string = msg_create((const char**) op_args + 1, op_args_cnt - 1);
            }
            else if (!strcmp(buffer, "delete")) {
                json_string = msg_delete((const char**) op_args + 1, op_args_cnt - 1);
            }
            else
                json_string = NULL;

            if (json_string == NULL) {
                printf("Wrong format\n");
                free(buffer);
                continue;
            }

            if (write(sfd, json_string, strlen(json_string)) == -1) {
                printf("Failed to send data to server\n");
            }

            if (read(sfd, &response, BUF_LENGTH) == -1) {
                printf("Failed to get a response\n");
            }

            print_json(response, buffer);

            memset(response, 0, BUF_LENGTH);
            free(buffer);
        }
    }

    exit(EXIT_SUCCESS);
}
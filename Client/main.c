#include <stdlib.h>
#include <fcntl.h>
#include <stdbool.h>
#include <signal.h>
#include <bits/sigaction.h>
#include "connection.h"
#include "json_parsing.h"
#include "readline_settings.h"

#define ARGS_MAX_LENGTH 10
#define BUF_LENGTH 1024

#define HOST_INDEX 1
#define PORT_INDEX 2

bool sig_abort = false;

void handler (int sig_num) {
    sig_abort = true;
}

int main(int argc, char** argv)
{
    int sfd, op_args_cnt;
    char *buffer, *json_string, response[BUF_LENGTH];
    char *op_args[ARGS_MAX_LENGTH];
    struct sigaction sa;

    if (argc < 3) {
        printf("Usage: %s host port", argv[0]);
        exit(EXIT_FAILURE);
    }

    sfd = connect_to_server(argv[HOST_INDEX], argv[PORT_INDEX]);

    if (sfd == -1) {
        printf("Could not connect to server\n");
        exit(EXIT_FAILURE);
    }

    fcntl(sfd, F_SETFL, O_NONBLOCK);

    sa.sa_handler = &handler;
    sigemptyset(&sa.sa_mask);
    sigaddset(&sa.sa_mask, SIGINT);

    if (sigaction(SIGINT, &sa, NULL) == -1) {
        printf("Failed to assign signal handler");
        exit(EXIT_FAILURE);
    }

    rl_attempted_completion_function = crud_completion;

    printf("Entered interactive mode\nFor help, type help\n");

    while (1) {
        buffer = readline(">> ");
        if (buffer) {

            add_history(buffer);

            op_args_cnt = get_op_args(buffer, op_args, ARGS_MAX_LENGTH);

            if (op_args_cnt == -1) {
                printf("Too many arguments\n");
                free(buffer);
                continue;
            }

            if (!strcmp(buffer, "help")) {
                printf("Command format: op_id msg_type args\nop_id: create/read/update/delete/find\n"
                       "msg_type: \n\tcoords, args - latitude longitude\n"
                       "\tperson args - first_name last_name age\n"
                       "Create:\n\tcreate msg_type device_id args\n"
                       "Read:\n\tread | read [ids]\n"
                       "Update:\n\tupdate msg_type msg_id field value [field value ...]"
                       "\n\tfields:\n\tcoords - lat, lon\n\tperson - fname, lname, age\n"
                       "Delete:\n\tdelete [ids]\n"
                       "Find:\n\tfind msg_type field value [field value ...]\n");
                free(buffer);
                continue;
            }

            if (!strcmp(buffer, "read")) {
                json_string = msg_read((const char**) (op_args + 1), op_args_cnt - 1);
            }
            else if (!strcmp(buffer, "create")) {
                json_string = msg_create((const char**) (op_args + 1), op_args_cnt - 1);
            }
            else if (!strcmp(buffer, "update")) {
                json_string = msg_update((const char**) (op_args+1), op_args_cnt - 1);
            }
            else if (!strcmp(buffer, "delete")) {
                json_string = msg_delete((const char**) (op_args + 1), op_args_cnt - 1);
            }
            else if (!strcmp(buffer, "find")) {
                json_string = msg_find((const char **) (op_args + 1), op_args_cnt - 1);
            }
            else if (!strcmp(buffer, "exit")) {
                json_string = get_op_json(OP_CLOSE);
                write(sfd, json_string, strlen(json_string));
                free(buffer);
                close(sfd);
                break;
            }
            else json_string = NULL;

            if (json_string == NULL) {
                printf("Wrong format\n");
                free(buffer);
                continue;
            }

            if (write(sfd, json_string, strlen(json_string)) == -1) {
                printf("Failed to send data to server\n");
            }

            sig_abort = false;

            while(read(sfd, &response, BUF_LENGTH) == -1) {
                if (sig_abort) {
                    sig_abort = false;
                    json_string = get_op_json(OP_ABORT);
                    write(sfd, json_string, strlen(json_string));
                    continue;
                }
            }

            print_json(response, buffer);

            memset(response, 0, BUF_LENGTH);
            free(json_string);
            free(buffer);
        }
    }

    exit(EXIT_SUCCESS);
}
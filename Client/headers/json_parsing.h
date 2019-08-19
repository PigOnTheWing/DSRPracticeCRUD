#ifndef CLIENT_CRUD_FUNCTIONS_H
#define CLIENT_CRUD_FUNCTIONS_H

#include <errno.h>
#include "messages.h"
#include "operations.h"

#define CREATE_MESSAGE_TYPE 0
#define CREATE_DEVICE_ID 1

#define CREATE_PERSON_FNAME 2
#define CREATE_PERSON_LNAME 3
#define CREATE_PERSON_AGE 4

#define CREATE_COORDS_LAT 2
#define CREATE_COORDS_LON 3

#define UPDATE_MESSAGE_TYPE 0
#define UPDATE_MESSAGE_ID 1
#define UPDATE_ARGS_START 2

#define FIND_MESSAGE_TYPE 0
#define FIND_ARGS_START 1

char *msg_create(const char **args, int arg_cnt);
char *msg_read(const char **args, int arg_cnt);
char *msg_update(const char **args, int arg_cnt);
char *msg_delete(const char **args, int arg_cnt);
char *msg_find(const char **args, int arg_cnt);

char *get_op_json(int op);

int print_json(char *json_string, const char * op);

#endif //CLIENT_CRUD_FUNCTIONS_H

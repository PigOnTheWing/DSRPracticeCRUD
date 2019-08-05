#ifndef CLIENT_CRUD_FUNCTIONS_H
#define CLIENT_CRUD_FUNCTIONS_H

#include <errno.h>
#include "../../lib/messages/messages.h"

#define OP_CREATE 1
#define OP_READ 2
#define OP_UPDATE 3
#define OP_DELETE 4

char *msg_create(const char **args, int arg_cnt);
char *msg_read(const char** args, int arg_cnt);
int msg_update(const char** args, const char** fields, int arg_cnt);
char *msg_delete(const char** args, int arg_cnt);

int print_json(char *json_string, const char * op);

#endif //CLIENT_CRUD_FUNCTIONS_H

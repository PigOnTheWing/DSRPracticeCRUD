#ifndef SERVER_CRUD_FUNCTIONS_H
#define SERVER_CRUD_FUNCTIONS_H

#include <time.h>
#include "crud_io.h"

#define OP_CREATE 1
#define OP_READ 2
#define OP_UPDATE 3
#define OP_DELETE 4

#define FILE_DATA_SIZE sizeof(struct file_data)
#define MESSAGE_BLOCK_SIZE sizeof(struct message_block)

long msg_create(json_t *json_msg, FILE *f);
long msg_read(json_t *ids, json_t *array, FILE *f);
long msg_delete(json_t *ids, FILE *f);

#endif //SERVER_CRUD_FUNCTIONS_H

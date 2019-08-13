#ifndef SERVER_CRUD_FUNCTIONS_H
#define SERVER_CRUD_FUNCTIONS_H

#include <time.h>
#include "crud_io.h"

long msg_create(struct message *msg, FILE *f);
long msg_read(const long *ids, size_t ids_size, int all, struct message *messages, FILE *f);
long msg_update(struct message *msg, FILE *f);
long msg_delete(const long *ids, size_t ids_size, FILE *f);

#endif //SERVER_CRUD_FUNCTIONS_H

#ifndef SERVER_CRUD_FUNCTIONS_H
#define SERVER_CRUD_FUNCTIONS_H

#include <time.h>
#include <errno.h>
#include "crud_io.h"

#define DB_OVERFLOW -2

long msg_create(struct message *msg, void *mmap_ptr, int m_block_count);
long msg_read(const long *ids, size_t ids_size, int all, struct message *messages, void *mmap_ptr);
long msg_update(struct message *msg, void *mmap_ptr);
long msg_delete(const long *ids, size_t ids_size, void *mmap_ptr);
long msg_find(struct message *msg, int comp_cnt, struct message *messages, size_t msgs_len, void *mmap_ptr);

#endif //SERVER_CRUD_FUNCTIONS_H

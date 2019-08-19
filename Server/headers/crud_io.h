#ifndef CRUD_CRUD_IO_H
#define CRUD_CRUD_IO_H

#include <stdbool.h>
#include "messages.h"

#define FILE_DATA_SIZE sizeof(struct file_data)
#define MESSAGE_BLOCK_SIZE sizeof(struct message_block)

struct file_data {
  long next_id;
  long count;
  size_t message_block_size;
};

struct message_block {
  struct message m;
  bool is_to_be_deleted;
};

#endif //CRUD_CRUD_IO_H

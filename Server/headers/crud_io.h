#ifndef CRUD_CRUD_IO_H
#define CRUD_CRUD_IO_H

#include <stdbool.h>
#include "../../lib/messages/messages.h"

struct file_data {
  long next_id;
  long count;
};

struct message_block {
  struct message m;
  bool is_to_be_deleted;
};

#endif //CRUD_CRUD_IO_H

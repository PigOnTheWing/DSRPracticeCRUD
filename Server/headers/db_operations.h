#ifndef CRUD_DB_OPERATIONS_H
#define CRUD_DB_OPERATIONS_H

#include <unistd.h>
#include "crud_io.h"

FILE *get_file(const char *filename);
int check_and_update(const char *filename);

#endif //CRUD_DB_OPERATIONS_H

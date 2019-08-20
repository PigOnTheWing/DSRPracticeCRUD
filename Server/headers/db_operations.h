#ifndef CRUD_DB_OPERATIONS_H
#define CRUD_DB_OPERATIONS_H

#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include "crud_io.h"

#define NUM_OF_MESSAGES 10

int get_file(const char *filename);
int check_and_update(const char *filename);
int get_mapping(int fd, void **mmap_ptr);
int resize_file(int fd, void **mmap_ptr, int *m_block_count);
int shrink_file(int fd, void **mmap_ptr);

#endif //CRUD_DB_OPERATIONS_H

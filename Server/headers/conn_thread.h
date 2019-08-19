#ifndef SERVER_CONN_THREAD_H
#define SERVER_CONN_THREAD_H

#include <netinet/in.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <fcntl.h>
#include "crud_functions.h"
#include "json_parsing.h"
#include "db_operations.h"

#define SIZE 1024
#define MESSAGES_MAX_SIZE 20

struct thread_info {
  pthread_t t_id;
  struct
  {
    int conn_fd;
    struct sockaddr_in* peer_addr;
    socklen_t* peer_len;
  } connection;
  struct
  {
    int fd;
    void **mmap_ptr;
    int *m_block_count;
  } db_data;
};

void *main_routine(void *arg);

#endif //SERVER_CONN_THREAD_H

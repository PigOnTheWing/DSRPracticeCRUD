#ifndef SERVER_CONN_THREAD_H
#define SERVER_CONN_THREAD_H

#include <netinet/in.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <fcntl.h>
#include "crud_functions.h"
#include "json_parsing.h"

#define SIZE 1024
#define MESSAGES_MAX_SIZE 20

struct thread_info {
  pthread_t t_id;
  int conn_fd;
  struct sockaddr_in *peer_addr;
  socklen_t *peer_len;
  FILE *f;
};

void *main_routine(void *arg);

#endif //SERVER_CONN_THREAD_H

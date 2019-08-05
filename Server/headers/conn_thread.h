#ifndef SERVER_CONN_THREAD_H
#define SERVER_CONN_THREAD_H

#include <netinet/in.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include "crud_functions.h"

#define SIZE 1024

struct thread_info {
  pthread_t t_id;
  int conn_fd;
  struct sockaddr_in *peer_addr;
  socklen_t *peer_len;
  FILE **pool;
  int pool_len;
};

FILE *get_file(FILE *f, FILE **pool, int pool_len);
int ret_file(FILE *f, FILE **pool, int pool_len);
void set_error_resp(json_t *json_resp, const char* message);
void *main_routine(void *arg);

#endif //SERVER_CONN_THREAD_H

#ifndef CRUD_JSON_PARSING_H
#define CRUD_JSON_PARSING_H

#include "messages.h"
#include "operations.h"

struct request_data {
  int op;
  struct message m;
  int all;
  long* ids;
  size_t ids_max_size;
  size_t ids_actual_size;
  int comp_cnt;
};

int parse_request(const char *request, struct request_data *data);
int parse_request_read_delete(json_t *obj, long *ids, size_t ids_max_size, int *all);
int parse_find(json_t *obj, struct message *msg, int *comp_cnt);
char *get_error_response(const char* error);
char *get_successful_read(struct message *messages, size_t msg_size);
char *get_successful_create(long id);
char *get_successful_update();
char *get_successful_delete();
char *get_successful_find(struct message *messages, size_t msg_size);

#endif //CRUD_JSON_PARSING_H

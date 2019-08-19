#ifndef MESSAGES_H
#define MESSAGES_H

#include <jansson.h>
#include <string.h>

#define MSG_COORDS 1
#define MSG_NAME 2

#define FIRST_LAST_NAME_LEN 20
#define DEVICE_ID_LEN 16
#define TIME_LEN 32

struct coords {
  double lat;
  double lon;
};

struct person {
  char fname[FIRST_LAST_NAME_LEN];
  char lname[FIRST_LAST_NAME_LEN];
  int age;
};

struct message {
  long message_id;
  int message_type;
  char device_id[DEVICE_ID_LEN];
  char time[TIME_LEN];

  union {
    struct coords c;
    struct person p;
  } msg_payload;
};

int get_message_json(json_t *obj, struct message *m);
int get_message_struct(struct message *m, json_t *obj);

int get_coords_json(json_t *obj, struct coords *c);
int get_coords_struct(struct coords *point, json_t *obj);

int get_name_json(json_t *obj, struct person *p);
int get_name_struct(struct person *p, json_t *obj);

int update_message(struct message *dest, struct message *source);
int compare_messages(struct message *m1, struct message *m2, int comp_cnt);

#endif //MESSAGES_H

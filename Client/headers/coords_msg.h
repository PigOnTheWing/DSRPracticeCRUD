#ifndef CLIENT_COORDS_MSG_H
#define CLIENT_COORDS_MSG_H

#include <jansson.h>

#define MSG_COORDS 1

int get_coords_json(json_t *obj, const char **args);

#endif //CLIENT_COORDS_MSG_H

#include "../headers/crud_functions.h"

char *msg_create(const char **args, int arg_cnt) {
    int status;
    json_t *obj = json_object();
    json_t *payload = json_object();
    json_object_set(obj, "op_id", json_integer(OP_CREATE));

    if (!strcmp(args[1], "coords")) {
        if (arg_cnt == 5) {
            json_object_set(obj, "message_type", json_integer(MSG_COORDS));
            json_object_set(obj, "device_id", json_string(args[2]));

            status = get_coords_json(payload, args + 2);
            if (!status) {
                json_object_set(obj, "payload", payload);
                return json_dumps(obj, 0);
            }
        }
    }

    return NULL;
}

char *msg_read(const char** args, int arg_cnt) {
    int ids_index;
    json_t *ids;
    json_t *obj = json_object();
    json_object_set(obj, "op_id", json_integer(OP_READ));

    if (arg_cnt > 1) {
        json_object_set(obj, "all", json_integer(0));
        ids = json_array();

        for (ids_index = 1; ids_index < arg_cnt; ids_index++) {
            json_array_append(ids, json_string(args[ids_index]));
        }

        json_object_set(obj, "ids", ids);
    }
    else {
        json_object_set(obj, "all", json_integer(1));
    }
    return json_dumps(obj, 0);
}

int msg_update(const char** args, const char** fields, int arg_cnt) {}

char *msg_delete(const char** args, int arg_cnt) {
    int ids_index;
    json_t *ids;
    json_t *obj = json_object();
    json_object_set(obj, "op_id", json_integer(OP_DELETE));

    if (arg_cnt > 1) {
        ids = json_array();

        for (ids_index = 1; ids_index < arg_cnt; ids_index++) {
            json_array_append(ids, json_string(args[ids_index]));
        }

        json_object_set(obj, "ids", ids);

        return json_dumps(obj, 0);
    }
    else
        return NULL;
}

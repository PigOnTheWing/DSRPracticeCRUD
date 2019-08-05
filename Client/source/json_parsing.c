#include <messages.h>
#include "../headers/json_parsing.h"

char *msg_create(const char **args, int arg_cnt) {
    int status;
    struct message msg;
    json_t *obj = json_object();
    json_object_set(obj, "op_id", json_integer(OP_CREATE));

    if (strlen(args[1]) >= DEVICE_ID_LEN)
        return NULL;

    msg.message_id = -1;
    strcpy(msg.device_id, args[1]);
    strcpy(msg.time, "");

    if (!strcmp(args[0], "coords")) {
        double lat, lon;

        if (arg_cnt != 4)
            return NULL;

        msg.message_type = MSG_COORDS;

        if (!(lat = strtod(args[2], NULL)) || !(lon = strtod(args[3], NULL)))
            return NULL;

        msg.msg_payload.c.lat = lat;
        msg.msg_payload.c.lon = lon;
    }
    else if (!strcmp(args[0], "person")) {
        int age;

        if (arg_cnt != 5)
            return NULL;

        msg.message_type = MSG_NAME;

        if (!(age = strtol(args[4], NULL, 10)) || errno == ERANGE)
            return NULL;

        strcpy(msg.msg_payload.p.fname, args[2]);
        strcpy(msg.msg_payload.p.lname, args[3]);
        msg.msg_payload.p.age = age;
    }
    else return NULL;

    status = get_message_json(obj, &msg);

    if (status == -1)
        return NULL;

    return json_dumps(obj, 0);
}

char *msg_read(const char** args, int arg_cnt) {
    int ids_index;
    long id;
    size_t all = 0;
    json_t *ids;
    json_t *obj = json_object();
    json_object_set(obj, "op_id", json_integer(OP_READ));

    if (arg_cnt == 0) {
        all = 1;
        goto done;
    }

    ids = json_array();

    for (ids_index = 0; ids_index < arg_cnt; ids_index++) {
        if (!(id = strtol(args[ids_index], NULL, 10)))
            return NULL;

        json_array_append(ids, json_integer(id));
    }

    json_object_set(obj, "ids", ids);

    done:
    json_object_set(obj, "all", json_integer(all));
    return json_dumps(obj, 0);
}

int msg_update(const char** args, const char** fields, int arg_cnt) {}

char *msg_delete(const char** args, int arg_cnt) {
    int ids_index;
    long id;
    json_t *ids;
    json_t *obj = json_object();
    json_object_set(obj, "op_id", json_integer(OP_DELETE));

    if (arg_cnt == 0)
        return NULL;

    ids = json_array();

    for (ids_index = 0; ids_index < arg_cnt; ids_index++) {
        if (!(id = strtol(args[ids_index], NULL, 10)))
            return NULL;

        json_array_append(ids, json_integer(id));
    }

    json_object_set(obj, "ids", ids);

    return json_dumps(obj, 0);
}

int print_json(char *json_string, const char * op) {
    struct message msg;
    size_t index;
    json_t *status, *err, *val, *read_arr, *value;
    json_t *obj = json_loads(json_string, 0, NULL);

    if (obj == NULL)
        return -1;

    status = json_object_get(obj, "status");
    if (json_equal(status, json_integer(0))) {
        err = json_object_get(obj, "error");
        printf("Error: %s\n", json_string_value(err));
        return 0;
    }

    if (!strcmp(op, "read")) {
        read_arr = json_object_get(obj, "messages");
        json_array_foreach(read_arr, index, value) {
            get_message_struct(&msg, value);
            printf("id=%ld\ndevice_id=%s\ntime=%s\n", msg.message_id, msg.device_id, msg.time);

            switch (msg.message_type) {
            case MSG_NAME:
                printf("first_name=%s\nlast_name=%s\nage=%d",
                        msg.msg_payload.p.fname,
                        msg.msg_payload.p.lname,
                        msg.msg_payload.p.age);
                break;
            case MSG_COORDS:
                printf("lat=%f\nlon=%f",
                        msg.msg_payload.c.lat,
                        msg.msg_payload.c.lon);
            }
        }
    }
    else if (!strcmp(op, "create")) {
        val = json_object_get(obj, "val");
        printf("Create successful, id:\n%lld", json_integer_value(val));
    }
    else {
        printf("Operation successful");
    }

    return 0;
}

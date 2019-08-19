#include "json_parsing.h"

char *msg_create(const char **args, int arg_cnt) {
    int status;
    char *json_msg;
    struct message msg;
    json_t *obj;

    if (strlen(args[CREATE_DEVICE_ID]) >= DEVICE_ID_LEN) {
        return NULL;
    }

    msg.message_id = -1;
    strcpy(msg.device_id, args[CREATE_DEVICE_ID]);
    strcpy(msg.time, "");

    if (!strcmp(args[CREATE_MESSAGE_TYPE], "coords")) {
        double lat, lon;

        if (arg_cnt != 4) {
            return NULL;
        }

        msg.message_type = MSG_COORDS;

        if (!(lat = strtod(args[CREATE_COORDS_LAT], NULL)) || !(lon = strtod(args[CREATE_COORDS_LON], NULL))) {
            return NULL;
        }

        msg.msg_payload.c.lat = lat;
        msg.msg_payload.c.lon = lon;
    }
    else if (!strcmp(args[CREATE_MESSAGE_TYPE], "person")) {
        int age;

        if (arg_cnt != 5) {
            return NULL;
        }

        msg.message_type = MSG_NAME;

        if (!(age = strtol(args[CREATE_PERSON_AGE], NULL, 10)) || errno == ERANGE) {
            return NULL;
        }

        strcpy(msg.msg_payload.p.fname, args[CREATE_PERSON_FNAME]);
        strcpy(msg.msg_payload.p.lname, args[CREATE_PERSON_LNAME]);
        msg.msg_payload.p.age = age;
    }
    else return NULL;

    obj = json_object();
    json_object_set_new(obj, "op_id", json_integer(OP_CREATE));

    status = get_message_json(obj, &msg);

    if (status == -1) {
        json_decref(obj);
        return NULL;
    }

    json_msg = json_dumps(obj, 0);
    json_decref(obj);

    return json_msg;
}

char *msg_read(const char** args, int arg_cnt) {
    int ids_index;
    long id;
    char *json_msg;
    json_t *ids;
    json_t *obj = json_object();
    json_object_set_new(obj, "op_id", json_integer(OP_READ));

    ids = json_array();

    if (arg_cnt == 0) {
        goto done;
    }

    for (ids_index = READ_ARGS_START; ids_index < arg_cnt; ids_index++) {
        if (!(id = strtol(args[ids_index], NULL, 10))) {
            json_decref(obj);
            json_decref(ids);
            return NULL;
        }

        json_array_append_new(ids, json_integer(id));
    }

    done:
    json_object_set_new(obj, "ids", ids);

    json_msg = json_dumps(obj, 0);
    json_decref(obj);

    return json_msg;
}

char *msg_update(const char** args, int arg_cnt) {
    int arg_index, status;
    long id;
    char *json_msg;
    json_t *obj;
    struct message msg;

    if (arg_cnt <= UPDATE_ARGS_START) {
        return NULL;
    }

    if (!(id = strtol(args[UPDATE_MESSAGE_ID], NULL, 10))) {
        return NULL;
    }

    msg.message_id = id;
    strcpy(msg.time, "");
    strcpy(msg.device_id, "");

    if (!strcmp(args[UPDATE_MESSAGE_TYPE], "coords")) {
        double lat = -1, lon = -1;

        msg.message_type = MSG_COORDS;

        for (arg_index = UPDATE_ARGS_START; arg_index < arg_cnt; arg_index += 2) {
            if (arg_index + 1 == arg_cnt) {
                return NULL;
            }

            if (!strcmp(args[arg_index], "lat")) {
                if (!(lat = strtod(args[arg_index + 1], NULL))) {
                    return NULL;
                }
            }
            else if (!strcmp(args[arg_index], "lon")) {
                if (!(lon = strtod(args[arg_index + 1], NULL))) {
                    return NULL;
                }
            }
            else return NULL;
        }

        msg.msg_payload.c.lat = lat;
        msg.msg_payload.c.lon = lon;
    }
    else if (!strcmp(args[UPDATE_MESSAGE_TYPE], "person")) {
        int age = -1;

        msg.message_type = MSG_NAME;

        strcpy(msg.msg_payload.p.fname, "");
        strcpy(msg.msg_payload.p.lname, "");

        for (arg_index = UPDATE_ARGS_START; arg_index < arg_cnt; arg_index += 2) {
            if (arg_index + 1 == arg_cnt) {
                return NULL;
            }

            if (!strcmp(args[arg_index], "age")) {
                if (!(age = strtol(args[arg_index + 1], NULL, 10)) || errno == ERANGE) {
                    return NULL;
                }
            }
            else if (!strcmp(args[arg_index], "fname")) {
                strcpy(msg.msg_payload.p.fname, args[arg_index + 1]);
            }
            else if (!strcmp(args[arg_index], "lname")) {
                strcpy(msg.msg_payload.p.lname, args[arg_index + 1]);
            }
            else return NULL;
        }

        msg.msg_payload.p.age = age;
    }
    else return NULL;

    obj = json_object();
    json_object_set_new(obj, "op_id", json_integer(OP_UPDATE));

    status = get_message_json(obj, &msg);

    if (status == -1) {
        json_decref(obj);
        return NULL;
    }

    json_msg = json_dumps(obj, 0);
    json_decref(obj);

    return json_msg;
}

char *msg_delete(const char** args, int arg_cnt) {
    int ids_index;
    long id;
    char *json_msg;
    json_t *ids;
    json_t *obj = json_object();
    json_object_set_new(obj, "op_id", json_integer(OP_DELETE));

    if (arg_cnt == 0) {
        return NULL;
    }

    ids = json_array();

    for (ids_index = DELETE_ARGS_START; ids_index < arg_cnt; ids_index++) {
        if (!(id = strtol(args[ids_index], NULL, 10))) {
            json_decref(obj);
            json_decref(ids);
            return NULL;
        }

        json_array_append_new(ids, json_integer(id));
    }

    json_object_set_new(obj, "ids", ids);

    json_msg = json_dumps(obj, 0);
    json_decref(obj);

    return json_msg;
}

char *msg_find(const char **args, int arg_cnt) {
    int status, arg_index, comparisons_count = 0;
    char *json_msg;
    json_t *obj;
    struct message msg;

    msg.message_id = -1;
    strcpy(msg.time, "");
    strcpy(msg.device_id, "");

    if (arg_cnt == FIND_ARGS_START) {
        return NULL;
    }

    for (arg_index = FIND_ARGS_START; arg_index < arg_cnt; arg_index += 2) {
        if (arg_index + 1 == arg_cnt) {
            return NULL;
        }

        comparisons_count++;

        if (!strcmp(args[arg_index], "id")) {
            long id;

            if (!(id = strtol(args[arg_index + 1], NULL, 10))) {
                return NULL;
            }

            msg.message_id = id;
        }
        else if (!strcmp(args[arg_index], "device_id")) {
            strcpy(msg.device_id, args[arg_index + 1]);
        }
    }

    if (!strcmp(args[FIND_MESSAGE_TYPE], "coords")) {
        double lat = -1, lon = -1;

        msg.message_type = MSG_COORDS;

        for (arg_index = FIND_ARGS_START; arg_index < arg_cnt; arg_index += 2) {

            if (!strcmp(args[arg_index], "lat")) {
                if (!(lat = strtod(args[arg_index + 1], NULL))) {
                    return NULL;
                }
            }
            else if (!strcmp(args[arg_index], "lon")) {
                if (!(lon = strtod(args[arg_index + 1], NULL))) {
                    return NULL;
                }
            }
        }

        msg.msg_payload.c.lat = lat;
        msg.msg_payload.c.lon = lon;
    }
    else if (!strcmp(args[FIND_MESSAGE_TYPE], "person")) {
        int age = -1;

        msg.message_type = MSG_NAME;

        strcpy(msg.msg_payload.p.fname, "");
        strcpy(msg.msg_payload.p.lname, "");

        for (arg_index = FIND_ARGS_START; arg_index < arg_cnt; arg_index += 2) {
            if (!strcmp(args[arg_index], "age")) {
                if (!(age = strtol(args[arg_index + 1], NULL, 10)) || errno == ERANGE) {
                    return NULL;
                }
            }
            else if (!strcmp(args[arg_index], "fname")) {
                strcpy(msg.msg_payload.p.fname, args[arg_index + 1]);
            }
            else if (!strcmp(args[arg_index], "lname")) {
                strcpy(msg.msg_payload.p.lname, args[arg_index + 1]);
            }
        }

        msg.msg_payload.p.age = age;
    }
    else return NULL;

    obj = json_object();
    json_object_set_new(obj, "op_id", json_integer(OP_FIND));
    json_object_set_new(obj, "c_cnt", json_integer(comparisons_count));

    status = get_message_json(obj, &msg);

    if (status == -1) {
        json_decref(obj);
        return NULL;
    }

    json_msg = json_dumps(obj, 0);
    json_decref(obj);

    return json_msg;
}

char *get_op_json(int op) {
    char *json_str;
    json_t *obj = json_object();

    json_object_set_new(obj, "op_id", json_integer(op));

    json_str = json_dumps(obj, 0);
    json_decref(obj);

    return json_str;
}

int print_json(char *json_string, const char * op) {
    struct message msg;
    size_t index;
    json_t *status, *err, *id, *read_arr, *value;
    json_t *obj = json_loads(json_string, 0, NULL);

    if (obj == NULL) {
        return -1;
    }

    status = json_object_get(obj, "status");
    if (json_equal(status, json_integer(0))) {
        err = json_object_get(obj, "error");
        printf("\nError: %s\n", json_string_value(err));
        return 0;
    }

    if (!strcmp(op, "read") || !strcmp(op, "find")) {
        read_arr = json_object_get(obj, "messages");
        json_array_foreach(read_arr, index, value) {
            get_message_struct(&msg, value);
            printf("id=%ld\ndevice_id=%s\ntime=%s", msg.message_id, msg.device_id, msg.time);

            switch (msg.message_type) {
            case MSG_NAME:
                printf("first_name=%s\nlast_name=%s\nage=%d\n\n",
                        msg.msg_payload.p.fname,
                        msg.msg_payload.p.lname,
                        msg.msg_payload.p.age);
                break;
            case MSG_COORDS:
                printf("lat=%f\nlon=%f\n\n",
                        msg.msg_payload.c.lat,
                        msg.msg_payload.c.lon);
            }
        }
    }
    else if (!strcmp(op, "create")) {
        id = json_object_get(obj, "id");
        printf("Create successful, id:\n%lld\n", json_integer_value(id));
    }
    else {
        printf("Operation successful\n");
    }

    json_decref(obj);
    return 0;
}

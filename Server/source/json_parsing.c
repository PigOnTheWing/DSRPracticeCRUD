#include "../headers/json_parsing.h"

int parse_request(const char *request, struct request_data *data) {
    int status = 0;
    json_t *json_req, *op;
    json_req = json_loads(request, 0, NULL);

    if (json_req == NULL)
        status = -1;

    op = json_object_get(json_req, "op_id");

    if (op == NULL)
        status = -1;

    data->op = json_integer_value(op);

    switch (data->op) {
    case OP_READ:
    case OP_DELETE:
        data->ids_actual_size = parse_request_read_delete(json_req, data->ids, data->ids_max_size, &data->all);
        break;
    case OP_CREATE:
    case OP_UPDATE:
        status = get_message_struct(&data->m, json_req);
        break;
    case OP_CLOSE:
    case OP_ABORT:
        break;
    default:
        status = -1;
    }

    json_decref(json_req);
    return status;
}

int parse_request_read_delete(json_t *obj, long *ids, size_t ids_max_size, int *all) {
    size_t index, id_count;
    json_t *id_array, *val;

    id_array = json_object_get(obj, "ids");

    if (id_array == NULL)
        return -1;

    id_count = json_array_size(id_array);

    if (id_count > ids_max_size)
        id_count = ids_max_size;

    if (id_count == 0) {
        id_count = ids_max_size;
        *all = 1;
    }

    json_array_foreach(id_array, index, val) {
        if (index == id_count)
            break;

        ids[index] = json_integer_value(val);
    }

    return id_count;
}

char *get_error_response(const char* error) {
    char *response;
    json_t *json_resp = json_object();

    json_object_set_new(json_resp, "status", json_integer(0));
    json_object_set_new(json_resp, "error", json_string(error));

    response = json_dumps(json_resp, 0);
    json_decref(json_resp);

    return response;
}

char *get_successful_read(struct message *messages, size_t msg_size) {
    size_t index;
    char *response;
    json_t *obj = json_object(), *array = json_array(), *msg;

    json_object_set_new(obj, "status", json_integer(1));

    for (index = 0; index < msg_size; index++) {
        msg = json_object();
        get_message_json(msg, &messages[index]);

        json_array_append_new(array, msg);
    }

    json_object_set_new(obj, "messages", array);

    response = json_dumps(obj, 0);
    json_decref(obj);

    return response;
}
char *get_successful_create(long id) {
    char *response;
    json_t *obj = json_object();

    json_object_set_new(obj, "status", json_integer(1));
    json_object_set_new(obj, "id", json_integer(id));

    response = json_dumps(obj, 0);
    json_decref(obj);

    return response;
}
char *get_successful_update() {
    char *response;
    json_t *obj = json_object();

    json_object_set_new(obj, "status", json_integer(1));

    response = json_dumps(obj, 0);
    json_decref(obj);

    return response;
}
char *get_successful_delete() {
    return get_successful_update();
}

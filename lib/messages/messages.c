#include "messages.h"

int get_message_json(json_t *obj, struct message *m) {
    int status;
    json_t *message_id, *message_type, *device_id, *time, *payload = json_object();

    message_id = json_integer(m->message_id);
    message_type = json_integer(m->message_type);
    device_id = json_string(m->device_id);
    time = json_string(m->time);

    if (!message_id || !message_type || !device_id || !time)
        return -1;

    json_object_set_new(obj, "message_id", message_id);
    json_object_set_new(obj, "message_type", message_type);
    json_object_set_new(obj, "device_id", device_id);
    json_object_set_new(obj, "time", time);

    switch(m->message_type) {
        case MSG_COORDS:
            status = get_coords_json(payload, &m->msg_payload.c);
            break;
        case MSG_NAME:
            status = get_name_json(payload, &m->msg_payload.p);
            break;
        default:return -1;
    }

    if (status == -1)
        return -1;

    json_object_set_new(obj, "payload", payload);

    return 0;
}

int get_message_struct(struct message *m, json_t *obj) {
    int status;
    json_t *j_message_id, *j_message_type, *j_device_id, *j_time, *j_payload;

    j_message_id = json_object_get(obj, "message_id");
    j_message_type = json_object_get(obj, "message_type");
    j_device_id = json_object_get(obj, "device_id");
    j_time = json_object_get(obj, "time");
    j_payload = json_object_get(obj, "payload");

    if (!j_message_id || !j_message_type || !j_device_id || !j_time || !j_payload)
        return -1;

    m->message_id = json_integer_value(j_message_id);
    m->message_type = json_integer_value(j_message_type);
    strcpy(m->device_id, json_string_value(j_device_id));
    strcpy(m->time, json_string_value(j_time));

    switch (m->message_type) {
        case MSG_COORDS:
            status = get_coords_struct(&m->msg_payload.c, j_payload);
            break;
        case MSG_NAME:
            status = get_name_struct(&m->msg_payload.p, j_payload);
            break;
        default:return -1;
    }

    return status;
}

int get_coords_json(json_t *obj, struct coords *c) {
    json_t *lat, *lon;

    lat = json_real(c->lat);
    lon = json_real(c->lon);

    if (!lat || !lon)
        return -1;

    json_object_set_new(obj, "lat", lat);
    json_object_set_new(obj, "lon", lon);

    return 0;
}

int get_coords_struct(struct coords *point, json_t *obj) {
    json_t *lat, *lon;

    lat = json_object_get(obj, "lat");
    lon = json_object_get(obj, "lon");

    if (!lat || !lon) {
        return -1;
    }

    point->lat = json_real_value(lat);
    point->lon = json_real_value(lon);

    return 0;
}

int get_name_json(json_t *obj, struct person *p) {
    json_t *fname, *lname, *age;

    fname = json_string(p->fname);
    lname = json_string(p->lname);
    age = json_integer(p->age);

    if (!fname || !lname || !age)
        return -1;

    json_object_set_new(obj, "fname", fname);
    json_object_set_new(obj, "lname", lname);
    json_object_set_new(obj, "age", age);

    return 0;
}

int get_name_struct(struct person *p, json_t *obj) {
    int age;
    const char *fname, *lname;
    json_t *j_fname, *j_lname, *j_age;

    j_fname = json_object_get(obj, "fname");
    j_lname = json_object_get(obj, "lname");
    j_age = json_object_get(obj, "age");

    if (!j_fname || !j_lname || !j_age)
        return -1;

    fname = json_string_value(j_fname);
    lname = json_string_value(j_lname);
    age = json_integer_value(j_age);

    if (!fname || !lname || !age)
        return -1;

    strcpy(p->fname, fname);
    strcpy(p->lname, lname);
    p->age = age;

    return 0;
}

int update_message(struct message *dest, struct message *source) {
    switch (source->message_type) {
    case MSG_NAME:
        if (source->msg_payload.p.age != -1)
            dest->msg_payload.p.age = source->msg_payload.p.age;

        if (strcmp(source->msg_payload.p.fname, "") != 0)
            strcpy(dest->msg_payload.p.fname, source->msg_payload.p.fname);

        if (strcmp(source->msg_payload.p.lname, "") != 0)
            strcpy(dest->msg_payload.p.lname, source->msg_payload.p.lname);
        break;
    case MSG_COORDS:
        if (source->msg_payload.c.lat != -1)
            dest->msg_payload.c.lat = source->msg_payload.c.lat;

        if (source->msg_payload.c.lon != -1)
            dest->msg_payload.c.lon = source->msg_payload.c.lon;
        break;
    default:return -1;
    }
    return 0;
}

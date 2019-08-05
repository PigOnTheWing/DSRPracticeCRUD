#include "../headers/crud_functions.h"

long msg_create(json_t *json_msg, FILE *f) {
    int status;
    char *t;
    time_t seconds;
    struct message msg;
    struct file_data data;
    struct message_block m_block, new_block;

    status = get_message_struct(&msg, json_msg);

    if (status == -1)
        return -1;

    time(&seconds);
    t = ctime(&seconds);

    strcpy(msg.time, t);

    if (fread(&data, FILE_DATA_SIZE, 1, f) != 1)
        return -1;

    msg.message_id = data.next_id++;
    data.count++;

    rewind(f);

    if (fwrite(&data, FILE_DATA_SIZE, 1, f) != 1)
        return -1;

    new_block.m = msg;
    new_block.is_to_be_deleted = false;
    new_block.is_last = true;

    while (!feof(f)) {
        if (fread(&m_block, MESSAGE_BLOCK_SIZE, 1, f) != 1)
            return -1;

        if (m_block.is_to_be_deleted) {
            fseek(f, -MESSAGE_BLOCK_SIZE, SEEK_CUR);
            new_block.is_last = m_block.is_last;
            break;
        }
    }

    if (fwrite(&new_block, MESSAGE_BLOCK_SIZE, 1, f) != 1) {
        return -1;
    }

    return msg.message_id;
}

long msg_read(json_t *ids, json_t *array, FILE *f) {
    int status;
    size_t index;
    struct file_data data;
    struct message_block m_block;
    json_t *msg, *value, *id_array;
    json_t *all = json_object_get(ids, "all");

    if (all == NULL)
        return -1;

    if (fread(&data, FILE_DATA_SIZE, 1, f) != 1)
        return -1;

    if (json_equal(all, json_integer(1))) {
        while(!feof(f)) {
            if (fread(&m_block, MESSAGE_BLOCK_SIZE, 1, f) != 1)
                return -1;

            if (m_block.is_to_be_deleted)
                continue;

            msg = json_object();

            status = get_message_json(msg, &m_block.m);

            if (status == -1)
                continue;

            json_array_append_new(array, msg);
        }
    }
    else {
        id_array = json_object_get(ids, "ids");

        if (id_array == NULL)
            return -1;

        while(!feof(f)) {
            if (fread(&m_block, MESSAGE_BLOCK_SIZE, 1, f) != 1)
                return -1;

            if (m_block.is_to_be_deleted)
                continue;

            json_array_foreach(id_array, index, value) {
                if (json_equal(value, json_integer(m_block.m.message_id))) {
                    msg = json_object();

                    status = get_message_json(msg, &m_block.m);

                    if (status == -1)
                        continue;

                    json_array_append_new(array, msg);
                    json_array_remove(id_array, index);
                }
            }
        }
    }
    return 0;
}

long msg_delete(json_t *ids, FILE *f) {
    bool delete = true;
    size_t index;
    size_t bool_size = sizeof(bool);
    struct file_data data;
    struct message_block m_block;
    json_t *value, *id_array = json_object_get(ids, "ids");

    if (id_array == NULL)
        return -1;

    if (fread(&data, FILE_DATA_SIZE, 1, f) != 1)
        return -1;

    while (!feof(f)) {
        if (fread(&m_block, MESSAGE_BLOCK_SIZE, 1, f) != 1)
            return -1;

        if (m_block.is_to_be_deleted)
            continue;

        json_array_foreach(id_array, index, value) {
            if (json_equal(value, json_integer(m_block.m.message_id))) {
                fseek(f, -2*bool_size, SEEK_CUR);

                if (fwrite(&delete, bool_size, 1, f) != 1)
                    return -1;

                fseek(f, bool_size, SEEK_CUR);
                json_array_remove(id_array, index);
            }
        }
    }
    return 0;
}

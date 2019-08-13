#include "../headers/crud_functions.h"

long msg_create(struct message *msg, FILE *f) {
    char *t;
    time_t seconds;
    struct file_data data;
    struct message_block m_block, new_block;

    time(&seconds);
    t = ctime(&seconds);

    strcpy(msg->time, t);

    if (fread(&data, FILE_DATA_SIZE, 1, f) != 1)
        return -1;

    msg->message_id = data.next_id++;
    data.count++;

    rewind(f);

    if (fwrite(&data, FILE_DATA_SIZE, 1, f) != 1)
        return -1;

    new_block.m = *msg;
    new_block.is_to_be_deleted = false;

    while (!feof(f)) {
        if (fread(&m_block, MESSAGE_BLOCK_SIZE, 1, f) != 1) {
            if (ferror(f))
                return -1;
            continue;
        }

        if (m_block.is_to_be_deleted) {
            fseek(f, -MESSAGE_BLOCK_SIZE, SEEK_CUR);
            break;
        }
    }

    if (fwrite(&new_block, MESSAGE_BLOCK_SIZE, 1, f) != 1) {
        return -1;
    }

    return msg->message_id;
}

long msg_read(const long *ids, size_t ids_size, int all, struct message *messages, FILE *f) {
    size_t ids_index = 0, msg_index = 0;
    struct file_data data;
    struct message_block m_block;

    if (fread(&data, FILE_DATA_SIZE, 1, f) != 1)
        return -1;

    while (ids_size != msg_index && !feof(f)) {
        if (fread(&m_block, MESSAGE_BLOCK_SIZE, 1, f) != 1) {
            if (ferror(f))
                return -1;
            continue;
        }

        if (m_block.is_to_be_deleted)
            continue;

        if (!all) {
            for (ids_index = 0; ids_index < ids_size; ids_index++) {
                if (m_block.m.message_id == ids[ids_index]) {
                    messages[msg_index++] = m_block.m;
                    break;
                }
            }
            continue;
        }

        messages[msg_index++] = m_block.m;
    }

    return msg_index;
}

long msg_update(struct message *msg, FILE *f) {
    long id;
    char *t;
    time_t seconds;
    struct file_data data;
    struct message_block m_block;

    id = msg->message_id;

    time(&seconds);
    t = ctime(&seconds);

    if (fread(&data, FILE_DATA_SIZE, 1, f) != 1)
        return -1;

    while (!feof(f)) {
        if (fread(&m_block, MESSAGE_BLOCK_SIZE, 1, f) != 1) {
            if (ferror(f))
                return -1;
            continue;
        }

        if (m_block.m.message_id == id) {
            if (m_block.is_to_be_deleted)
                return -1;

            strcpy(m_block.m.time, t);

            if (update_message(&m_block.m, msg) == -1)
                return -1;

            fseek(f, -MESSAGE_BLOCK_SIZE, SEEK_CUR);

            if (fwrite(&m_block, MESSAGE_BLOCK_SIZE, 1, f) != 1)
                return -1;

            break;
        }
    }

    return 0;
}

long msg_delete(const long *ids, size_t ids_size, FILE *f) {
    size_t index = 0;
    struct file_data data;
    struct message_block m_block;

    if (fread(&data, FILE_DATA_SIZE, 1, f) != 1)
        return -1;

    while (!feof(f)) {
        if (fread(&m_block, MESSAGE_BLOCK_SIZE, 1, f)!=1) {
            if (ferror(f))
                return -1;
            continue;
        }

        if (m_block.is_to_be_deleted)
            continue;

        for (index = 0; index < ids_size; index++) {
            if (m_block.m.message_id == ids[index]) {
                m_block.is_to_be_deleted = true;

                fseek(f, -MESSAGE_BLOCK_SIZE, SEEK_CUR);

                if (fwrite(&m_block, MESSAGE_BLOCK_SIZE, 1, f)!=1)
                    return -1;

                data.count--;

                break;
            }
        }
    }

    rewind(f);
    fwrite(&data, FILE_DATA_SIZE, 1, f);

    return 0;
}

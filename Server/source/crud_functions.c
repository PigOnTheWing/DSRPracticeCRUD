#include "crud_functions.h"

long msg_create(struct message *msg, void *mmap_ptr, int m_block_count) {
    size_t count;
    char *t;
    time_t seconds;
    struct file_data *data;
    struct message_block *m_block, new_block;

    errno = 0;

    time(&seconds);
    t = ctime(&seconds);

    strcpy(msg->time, t);

    data = mmap_ptr;
    m_block = (struct message_block*) (data + 1);

    count = data->count;

    if (count == m_block_count) {
        errno = DB_OVERFLOW;
        return -1;
    }

    msg->message_id = data->next_id++;
    data->count++;

    new_block.m = *msg;
    new_block.is_to_be_deleted = false;

    for (int i = 0; i < count; i++) {
        if (m_block->is_to_be_deleted) {
            data->count--;
            break;
        }
        m_block++;
    }

    *m_block = new_block;

    return msg->message_id;
}

long msg_read(const long *ids, size_t ids_size, int all, struct message *messages, void *mmap_ptr) {
    size_t ids_index = 0, msg_index = 0, count;
    struct file_data *data;
    struct message_block *m_block;

    data = mmap_ptr;
    m_block = (struct message_block*) (data + 1);

    count = data->count;

    for (int i = 0; i < count; i++) {
        if (msg_index == ids_size) {
            break;
        }

        if (m_block->is_to_be_deleted) {
            m_block++;
            continue;
        }

        if (!all) {
            for (ids_index = 0; ids_index < ids_size; ids_index++) {
                if (m_block->m.message_id == ids[ids_index]) {
                    messages[msg_index++] = m_block->m;
                    break;
                }
            }
            m_block++;
            continue;
        }
        messages[msg_index++] = m_block->m;

        m_block++;
    }

    return msg_index;
}

long msg_update(struct message *msg, void *mmap_ptr) {
    int status = -1;
    long id;
    size_t count;
    char *t;
    time_t seconds;
    struct file_data *data;
    struct message_block *m_block;

    id = msg->message_id;

    time(&seconds);
    t = ctime(&seconds);

    data = mmap_ptr;
    m_block = (struct message_block*) (data + 1);

    count = data->count;

    for (int i = 0; i < count; i++) {
        if (m_block->is_to_be_deleted) {
            m_block++;
            continue;
        }

        if (m_block->m.message_id == id) {
            if (m_block->is_to_be_deleted) {
                return -1;
            }

            if (update_message(&m_block->m, msg) == -1) {
                return -1;
            }

            strcpy(m_block->m.time, t);
            status = 0;

            break;
        }
        m_block++;
    }

    return status;
}

long msg_delete(const long *ids, size_t ids_size, void *mmap_ptr) {
    size_t index = 0, count;
    struct file_data *data;
    struct message_block *m_block;

    data = mmap_ptr;
    m_block = (struct message_block*) (data + 1);

    count = data->count;

    for (int i = 0; i < count; i++) {
        if (m_block->is_to_be_deleted) {
            m_block++;
            continue;
        }

        for (index = 0; index < ids_size; index++) {
            if (m_block->m.message_id == ids[index]) {
                m_block->is_to_be_deleted = true;
                break;
            }
        }
        m_block++;
    }

    return 0;
}

long msg_find(struct message *msg, int comp_cnt, struct message *messages, size_t msgs_len, void *mmap_ptr) {
    size_t msg_index = 0, count;
    struct file_data *data;
    struct message_block *m_block;

    data = mmap_ptr;
    m_block = (struct message_block*) (data + 1);

    count = data->count;

    if (data->next_id < msg->message_id) {
        return -1;
    }

    for (int i = 0; i < count; i++) {
        if (m_block->is_to_be_deleted) {
            m_block++;
            i--;
            continue;
        }

        if (!compare_messages(msg, &m_block->m, comp_cnt)) {
            messages[msg_index++] = m_block->m;

            if (msg_index == msgs_len) {
                return msg_index;
            }
        }
        m_block++;
    }
    return msg_index;
}

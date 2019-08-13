#include "../headers/db_operations.h"

FILE *get_file(const char* filename) {
    FILE *f;
    struct file_data data;
    size_t m_b_size = sizeof(struct message_block);

    if (access(filename, R_OK|W_OK) == -1) {
        f = fopen(filename, "wb");

        if (f == NULL)
            return NULL;

        data.count = 0;
        data.next_id = 1;
        data.message_block_size = m_b_size;

        if (fwrite(&data, FILE_DATA_SIZE, 1, f) != 1)
            return NULL;

        fclose(f);
    }
    else {
        if (check_and_update(filename) == -1)
            return NULL;
    }

    return fopen(filename, "r+b");
}

int check_and_update(const char *filename) {
    const char *tmp_filename = "tmp.db";
    FILE *f, *tmp;
    struct file_data data;
    struct message_block m_block;
    size_t m_b_size = sizeof(struct message_block);

    f = fopen(filename, "rb");

    if (f == NULL)
        return -1;

    if (fread(&data, FILE_DATA_SIZE, 1, f) != 1)
        return -1;

    if (data.message_block_size == m_b_size)
        return 0;

    tmp = fopen(tmp_filename, "wb");

    if (tmp == NULL)
        return -1;

    while(!feof(f)) {
        if (fread(&m_block, data.message_block_size, 1, f) != 1) {
            if (ferror(f))
                return -1;
            continue;
        }

        if (fwrite(&m_block, MESSAGE_BLOCK_SIZE, 1, tmp) != 1)
            return -1;
    }

    fclose(f);
    fclose(tmp);

    if (remove(filename) == -1)
        return -1;

    if (rename(tmp_filename, filename) == -1)
        return -1;

    return 0;
}

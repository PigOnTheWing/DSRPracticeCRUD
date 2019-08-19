#include "db_operations.h"

int get_file(const char* filename) {
    FILE *f;
    struct file_data data;
    size_t m_b_size = sizeof(struct message_block);

    if (access(filename, R_OK|W_OK) == -1) {
        f = fopen(filename, "wb");

        if (f == NULL) {
            return -1;
        }

        data.count = 0;
        data.next_id = 1;
        data.message_block_size = m_b_size;

        if (fwrite(&data, FILE_DATA_SIZE, 1, f) != 1) {
            return -1;
        }

        fclose(f);
    }
    else {
        if (check_and_update(filename) == -1) {
            return -1;
        }
    }

    return open(filename, O_RDWR);
}

int check_and_update(const char *filename) {
    const char *tmp_filename = "tmp.db";
    FILE *f, *tmp;
    struct file_data data;
    struct message_block m_block;
    size_t m_b_size = sizeof(struct message_block);

    f = fopen(filename, "rb");

    if (f == NULL) {
        return -1;
    }

    if (fread(&data, FILE_DATA_SIZE, 1, f) != 1) {
        return -1;
    }

    if (data.message_block_size == m_b_size) {
        return 0;
    }

    tmp = fopen(tmp_filename, "wb");

    if (tmp == NULL) {
        return -1;
    }

    while(!feof(f)) {
        if (fread(&m_block, data.message_block_size, 1, f) != 1) {
            if (ferror(f)) {
                return -1;
            }
            continue;
        }

        if (fwrite(&m_block, MESSAGE_BLOCK_SIZE, 1, tmp) != 1) {
            return -1;
        }
    }

    fclose(f);
    fclose(tmp);

    if (remove(filename) == -1) {
        return -1;
    }

    if (rename(tmp_filename, filename) == -1) {
        return -1;
    }

    return 0;
}

int get_mapping(int fd, void **mmap_ptr) {
    int len, m_block_count;
    struct stat db_stat;

    if (fstat(fd, &db_stat) == -1) {
        return -1;
    }

    len = db_stat.st_size + (NUM_OF_MESSAGES * MESSAGE_BLOCK_SIZE);
    m_block_count = ((db_stat.st_size - FILE_DATA_SIZE) / MESSAGE_BLOCK_SIZE) + NUM_OF_MESSAGES;

    if (ftruncate(fd, len) == -1) {
        return -1;
    }

    *mmap_ptr = mmap(NULL, len, PROT_READ|PROT_WRITE, MAP_SHARED, fd, 0);

    if (*mmap_ptr == MAP_FAILED) {
        return -1;
    }

    return m_block_count;
}

int resize_file(int fd, void **mmap_ptr, int *m_block_count) {
    int len;
    void *ptr;

    len = (*m_block_count * MESSAGE_BLOCK_SIZE) + FILE_DATA_SIZE;
    if (msync(*mmap_ptr, len, MS_SYNC) == -1) {
        return -1;
    }

    if (munmap(*mmap_ptr, len) == -1) {
        return -1;
    }

    len += NUM_OF_MESSAGES * MESSAGE_BLOCK_SIZE;
    if (ftruncate(fd, len) == -1) {
        return -1;
    }

    ptr = mmap(NULL, len, PROT_READ|PROT_WRITE, MAP_SHARED, fd, 0);
    if (ptr == MAP_FAILED) {
        return -1;
    }

    *mmap_ptr = ptr;
    *m_block_count += NUM_OF_MESSAGES;
    return 0;
}

int shrink_file(int fd, void **mmap_ptr) {
    int msg_count, len;
    struct file_data *data;
    struct stat db_stat;

    data = *mmap_ptr;
    msg_count = data->count;

    if (fstat(fd, &db_stat) == -1) {
        return -1;
    }

    len = db_stat.st_size;

    if (msync(*mmap_ptr, len, MS_SYNC) == -1) {
        return -1;
    }

    if (munmap(*mmap_ptr, len) == -1) {
        return -1;
    }

    len = (msg_count * MESSAGE_BLOCK_SIZE) + FILE_DATA_SIZE;
    if (ftruncate(fd, len) == -1) {
        return -1;
    }

    close(fd);
    return 0;
}

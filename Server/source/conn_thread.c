#include "../headers/conn_thread.h"

pthread_mutex_t read_mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t read_cond = PTHREAD_COND_INITIALIZER;
bool free_to_read = true;

pthread_mutex_t file_mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t file_cond = PTHREAD_COND_INITIALIZER;
bool file_available = true;

FILE *get_file(FILE *f, FILE **pool, int pool_len) {
    int index;
    if (pthread_mutex_lock(&file_mutex))
        return NULL;

    do {
        for (index = 0; index < pool_len; index++) {
            if (pool[index] != NULL) {
                f = pool[index];
                pool[index] = NULL;
                break;
            }
        }

        if (index == pool_len) {
            file_available = false;
            while (!file_available)
                pthread_cond_wait(&file_cond, &file_mutex);
        }
    }
    while (index == pool_len);

    if (pthread_mutex_unlock(&file_mutex))
        return NULL;

    return f;
}

int ret_file(FILE *f, FILE **pool, int pool_len) {
    int index;
    if (pthread_mutex_lock(&file_mutex))
        return -1;

    for (index = 0; index < pool_len; index++) {
        if (pool[index] == NULL) {
            pool[index] = f;
            break;
        }
    }

    file_available = true;
    pthread_cond_signal(&file_cond);

    if (pthread_mutex_unlock(&file_mutex))
        return -1;

    return 0;
}

void set_error_resp(json_t *json_resp, const char* message) {
    json_object_set(json_resp, "status", json_integer(0));
    json_object_set(json_resp, "error", json_string(message));
}

void *main_routine(void *arg) {
    int status = 0;
    FILE *f = NULL;
    char BUF[SIZE];
    struct thread_info *info = arg;
    json_t *op, *json_req, *json_resp = json_object(), *read_array;
    json_error_t *json_err = NULL;

    f = get_file(f, info->pool, info->pool_len);

    while (1) {
        if (recvfrom(info->conn_fd, BUF, SIZE, 0, (struct sockaddr*) info->peer_addr, info->peer_len) > 0) {
            if (!strcmp(BUF, "close"))
                break;

            json_req = json_loads(BUF, 0, json_err);

            if (json_req == NULL) {
                set_error_resp(json_resp, "Failed to read json request");
                goto send;
            }

            op = json_object_get(json_req, "op_id");

            if (json_equal(op, json_integer(OP_READ))) {
                if (!free_to_read) {
                    pthread_mutex_lock(&read_mutex);

                    while (!free_to_read)
                        pthread_cond_wait(&read_cond, &read_mutex);

                    pthread_mutex_unlock(&read_mutex);
                }

                read_array = json_array();
                status = msg_read(json_req, read_array, f);

                if (status == -1) {
                    set_error_resp(json_resp, "Failed to read message(s)");
                    goto send;
                }

                json_object_set(json_resp, "messages", read_array);
            }
            else {
                pthread_mutex_lock(&read_mutex);
                free_to_read = false;

                if (json_equal(op, json_integer(OP_CREATE))) {
                    status = msg_create(json_req, f);
                }
                else if (json_equal(op, json_integer(OP_DELETE))) {
                    status = msg_delete(json_req, f);
                }

                free_to_read = true;
                pthread_cond_broadcast(&read_cond);
                pthread_mutex_unlock(&read_mutex);

                if (status == -1) {
                    set_error_resp(json_resp, "Failed to do operation");
                    goto send;
                }

                json_object_set(json_resp, "val", json_integer(status));
            }

            json_object_set(json_resp, "status", json_integer(1));

            send:
            sendto(info->conn_fd, json_dumps(json_resp, 0), SIZE, 0,
                    (const struct sockaddr*) info->peer_addr, *info->peer_len);

        }

        rewind(f);
        memset(BUF, 0, SIZE);
    }

    ret_file(f, info->pool, info->pool_len);

    info->pool = NULL;
    close(info->conn_fd);
    free(info);

    return NULL;
}

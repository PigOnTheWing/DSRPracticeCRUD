#include "../headers/conn_thread.h"

pthread_mutex_t op_mutex = PTHREAD_MUTEX_INITIALIZER;

void *main_routine(void *arg) {
    int status, read_cnt;
    long ids[MESSAGES_MAX_SIZE], id;
    FILE *f;
    char BUF[SIZE], *response = NULL;
    struct thread_info *info = arg;
    struct request_data data, abort_data;
    struct message messages[MESSAGES_MAX_SIZE];

    fcntl(info->conn_fd, F_SETFL, O_NONBLOCK);

    f = info->f;

    data.ids = ids;
    data.ids_max_size = MESSAGES_MAX_SIZE;
    data.all = 0;

    while (1) {
        if (recvfrom(info->conn_fd, BUF, SIZE, 0, (struct sockaddr*) info->peer_addr, info->peer_len) > 0) {
            if (parse_request(BUF, &data) == -1) {
                response = get_error_response("Failed to read json request");
                goto send;
            }

            if (data.op == OP_CLOSE)
                break;

            while (pthread_mutex_trylock(&op_mutex) != 0) {
                memset(BUF, 0, SIZE);

                if (recvfrom(info->conn_fd, BUF, SIZE, 0, (struct sockaddr*) info->peer_addr, info->peer_len) > 0) {
                    parse_request(BUF, &abort_data);
                    if (abort_data.op == OP_ABORT) {
                        response = get_error_response("Aborted");
                        goto send;
                    }
                }
            }

            switch (data.op) {
            case OP_CREATE:
                rewind(f);

                id = msg_create(&data.m, f);

                pthread_mutex_unlock(&op_mutex);

                if (id == -1) {
                    response = get_error_response("Failed to create a message");
                    goto send;
                }
                response = get_successful_create(id);
                break;
            case OP_READ:
                rewind(f);

                read_cnt = msg_read(data.ids, data.ids_actual_size, data.all, messages, f);

                pthread_mutex_unlock(&op_mutex);

                if (read_cnt == -1) {
                    response = get_error_response("Failed to read message(s)");
                    goto send;
                }
                response = get_successful_read(messages, read_cnt);
                break;
            case OP_UPDATE:
                rewind(f);

                status = msg_update(&data.m, f);

                pthread_mutex_unlock(&op_mutex);

                if (status == -1) {
                    response = get_error_response("Failed to update a message");
                    goto send;
                }
                response = get_successful_update();
                break;
            case OP_DELETE:
                rewind(f);

                status = msg_delete(data.ids, data.ids_actual_size, f);

                pthread_mutex_unlock(&op_mutex);

                if (status == -1) {
                    response = get_error_response("Failed to delete message(s)");
                    goto send;
                }
                response = get_successful_delete();
                break;
            default:continue;
            }

            send:
            sendto(info->conn_fd, response, SIZE, 0,
                    (const struct sockaddr*) info->peer_addr, *info->peer_len);

            free(response);
            memset(BUF, 0, SIZE);
        }
    }

    info->f = NULL;
    close(info->conn_fd);
    free(info);

    return NULL;
}

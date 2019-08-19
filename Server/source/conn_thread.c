#include <conn_thread.h>
#include "conn_thread.h"

#define log(t_id, op, status)\
    printf("Thread %lu:\n\tfile - %s, function - %s, line - %d:\n\top - %s, status - %s\n", t_id, __FILE__, __func__, __LINE__, op, status)

pthread_mutex_t op_mutex = PTHREAD_MUTEX_INITIALIZER;

void *main_routine(void *arg) {
    int status, m_block_count;
    size_t read_cnt, find_cnt;
    long ids[MESSAGES_MAX_SIZE], id;
    void *mmap_ptr;
    char BUF[SIZE], *response = NULL;
    struct thread_info *info = arg;
    struct request_data data, abort_data;
    struct message messages[MESSAGES_MAX_SIZE];

    fcntl(info->connection.conn_fd, F_SETFL, O_NONBLOCK);

    mmap_ptr = *info->db_data.mmap_ptr;

    data.ids = ids;
    data.ids_max_size = MESSAGES_MAX_SIZE;
    data.all = 0;

    while (1) {
        if (recvfrom(info->connection.conn_fd, BUF, SIZE, 0, (struct sockaddr*) info->connection.peer_addr, info->connection.peer_len) > 0) {
            if (parse_request(BUF, &data) == -1) {
                response = get_error_response("Failed to read json request");
                goto send;
            }

            if (data.op == OP_CLOSE) {
                break;
            }

            while (pthread_mutex_trylock(&op_mutex) != 0) {
                memset(BUF, 0, SIZE);

                if (recvfrom(info->connection.conn_fd, BUF, SIZE, 0, (struct sockaddr*) info->connection.peer_addr, info->connection.peer_len) > 0) {
                    parse_request(BUF, &abort_data);
                    if (abort_data.op == OP_ABORT) {
                        response = get_error_response("Aborted");
                        log(info->t_id, "abort", "success");
                        goto send;
                    }
                }
            }

            switch (data.op) {
            case OP_CREATE:
                id = msg_create(&data.m, mmap_ptr, *info->db_data.m_block_count);

                pthread_mutex_unlock(&op_mutex);

                if (id == -1) {
                    if (errno == DB_OVERFLOW) {
                        status = resize_file(info->db_data.fd, info->db_data.mmap_ptr, info->db_data.m_block_count);
                        if (status == -1) {
                            response = get_error_response("Create is unavailable");
                            log(info->t_id, "create", "db overflow");
                            goto send;
                        }
                    }

                    response = get_error_response("Failed to create a message");
                    log(info->t_id, "create", "failure");
                    goto send;
                }
                response = get_successful_create(id);
                log(info->t_id, "create", "success");
                break;
            case OP_READ:
                read_cnt = msg_read(data.ids, data.ids_actual_size, data.all, messages, mmap_ptr);

                pthread_mutex_unlock(&op_mutex);

                if (read_cnt == -1) {
                    response = get_error_response("Failed to read message(s)");
                    log(info->t_id, "read", "failure");
                    goto send;
                }
                response = get_successful_read(messages, read_cnt);
                log(info->t_id, "read", "success");
                break;
            case OP_UPDATE:
                status = msg_update(&data.m, mmap_ptr);

                pthread_mutex_unlock(&op_mutex);

                if (status == -1) {
                    response = get_error_response("Failed to update a message");
                    log(info->t_id, "update", "failure");
                    goto send;
                }
                response = get_successful_update();
                log(info->t_id, "update", "success");
                break;
            case OP_DELETE:
                status = msg_delete(data.ids, data.ids_actual_size, mmap_ptr);

                pthread_mutex_unlock(&op_mutex);

                if (status == -1) {
                    response = get_error_response("Failed to delete message(s)");
                    log(info->t_id, "delete", "failure");
                    goto send;
                }
                response = get_successful_delete();
                log(info->t_id, "delete", "success");
                break;
            case OP_FIND:
                find_cnt = msg_find(&data.m, data.comp_cnt, messages, MESSAGES_MAX_SIZE, mmap_ptr);

                pthread_mutex_unlock(&op_mutex);

                if (find_cnt == -1 || find_cnt == 0) {
                    response = get_error_response("Failed to find message(s)");
                    log(info->t_id, "find", "failure");
                    goto send;
                }
                response = get_successful_find(messages, find_cnt);
                log(info->t_id, "find", "success");
                break;
            default:continue;
            }

            send:
            sendto(info->connection.conn_fd, response, SIZE, 0,
                    (const struct sockaddr*) info->connection.peer_addr, *info->connection.peer_len);

            free(response);
            memset(BUF, 0, SIZE);
        }
    }

    log(info->t_id, "exit", "success");

    info->db_data.mmap_ptr = NULL;
    close(info->connection.conn_fd);
    free(info);

    return NULL;
}

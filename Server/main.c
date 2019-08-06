#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <stdlib.h>
#include <arpa/inet.h>
#include <netdb.h>
#include "headers/conn_thread.h"

#define BACKLOG 10
#define POOL_SIZE 10

void exit_with_error(const char* message) {
    printf("Error: %s", message);
    exit(EXIT_FAILURE);
}

int get_socket(char *host, char *port) {
    int sock_fd = -1, status, reuse_enable = 1;
    struct addrinfo hints;
    struct addrinfo *addr_list, *ai;

    if (host == NULL)
        host = "localhost";

    if (port == NULL)
        port = "8080";


    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_protocol = 0;
    hints.ai_flags = 0;
    hints.ai_canonname = NULL;
    hints.ai_addr = NULL;
    hints.ai_next = NULL;

    status = getaddrinfo(host, port, &hints, &addr_list);
    if (status)
        exit_with_error("could not get a socket");

    for (ai = addr_list; ai != NULL; ai = ai->ai_next) {
        sock_fd = socket(ai->ai_family, ai->ai_socktype, ai->ai_protocol);

        if (sock_fd == -1)
            continue;

        if (setsockopt(sock_fd, SOL_SOCKET, SO_REUSEADDR, &reuse_enable, sizeof(int)) == -1)
            exit_with_error("failed to set SO_REUSEADDR");

        if (bind(sock_fd, ai->ai_addr, ai->ai_addrlen) == 0)
            break;

        close(sock_fd);
    }

    if (ai == NULL)
        exit_with_error("bind failed");

    freeaddrinfo(addr_list);

    printf("host=%s\nport=%s\n", host, port);

    return sock_fd;
}

int get_pool(const char *filename, FILE **pool, int pool_len) {
    FILE *f;
    struct file_data data;

    if (access(filename, R_OK|W_OK) == -1) {
        f = fopen(filename, "wb");

        if (f == NULL)
            return -1;

        data.count = 0;
        data.next_id = 1;

        if (fwrite(&data, sizeof(data), 1, f) != 1)
            return -1;

        fclose(f);
    }

    for (int i = 0; i < pool_len; i++) {
        pool[i] = fopen(filename, "r+b");
    }

    return 0;
}

int main(int argc, char** argv)
{
    int sock_fd, conn_fd, info_size, opt;
    char *host = NULL, *port = NULL;
    socklen_t peer_len;
    struct sockaddr_in peer_addr;
    struct thread_info *info;
    FILE *pool[POOL_SIZE];

    if (argc < 4) {
        printf("Usage: %s filename [-h host]|[-p port]\n"
               "filename - name of file that acts as a database\n"
               "host - ipv4 address\n"
               "port - number of port to be listening\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    while ((opt = getopt(argc, argv, "h:p:")) != -1) {
        switch (opt) {
        case 'h':
            host = optarg;
            break;
        case 'p':
            port = optarg;
            break;
        default:
            printf("Usage: %s filename [-h host]|[-p port]\n"
                   "filename - name of file that acts as a database\n"
                   "host - ipv4 address\n"
                   "port - number of port to be listening\n", argv[0]);
            exit(EXIT_FAILURE);
        }
    }

    if (get_pool(argv[1], pool, POOL_SIZE) == -1) {
        exit_with_error("Could not find/create file");
    }

    sock_fd = get_socket(host, port);

    if (listen(sock_fd, BACKLOG) == -1)
        exit_with_error("could not listen to a port");

    peer_len = sizeof(struct sockaddr_in);
    info_size = sizeof(struct thread_info);

    while (1) {
        conn_fd = accept(sock_fd, (struct sockaddr*) &peer_addr, &peer_len);
        if (conn_fd == -1)
            exit_with_error("could not establish connection");


        info = NULL;
        info = malloc(info_size);

        info->conn_fd = conn_fd;
        info->peer_addr = &peer_addr;
        info ->peer_len = &peer_len;
        info->pool = pool;
        info->pool_len = POOL_SIZE;

        if (pthread_create(&info->t_id, NULL, &main_routine, info)) {
            free(info);
            exit(EXIT_FAILURE);
        }
    }

    exit(EXIT_SUCCESS);
}
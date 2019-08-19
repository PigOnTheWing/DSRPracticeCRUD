#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <stdlib.h>
#include <arpa/inet.h>
#include <netdb.h>
#include "conn_thread.h"
#include "db_operations.h"

#define BACKLOG 10
#define exit_with_error(message)\
    printf("Error: %s", message); exit(EXIT_FAILURE)

#define log(message)\
    printf("file - %s, function - %s, line - %d:\n\t%s\n", __FILE__, __func__, __LINE__, message)

int get_socket(char *host, char *port) {
    int sock_fd = -1, status, reuse_enable = 1;
    struct addrinfo hints;
    struct addrinfo *addr_list, *ai;

    if (host == NULL) {
        host = "localhost";
    }

    if (port == NULL) {
        port = "8080";
    }

    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_protocol = 0;
    hints.ai_flags = 0;
    hints.ai_canonname = NULL;
    hints.ai_addr = NULL;
    hints.ai_next = NULL;

    status = getaddrinfo(host, port, &hints, &addr_list);
    if (status) {
        exit_with_error("could not get a socket");
    }

    for (ai = addr_list; ai != NULL; ai = ai->ai_next) {
        sock_fd = socket(ai->ai_family, ai->ai_socktype, ai->ai_protocol);

        if (sock_fd == -1) {
            continue;
        }

        if (setsockopt(sock_fd, SOL_SOCKET, SO_REUSEADDR, &reuse_enable, sizeof(int)) == -1) {
            exit_with_error("failed to set SO_REUSEADDR");
        }

        if (bind(sock_fd, ai->ai_addr, ai->ai_addrlen) == 0) {
            break;
        }

        close(sock_fd);
    }

    if (ai == NULL) {
        exit_with_error("bind failed");
    }

    freeaddrinfo(addr_list);

    printf("host=%s\nport=%s\n", host, port);

    return sock_fd;
}

int main(int argc, char** argv)
{
    int sock_fd, conn_fd, info_size, opt;
    char *host = NULL, *port = NULL;
    socklen_t peer_len;
    struct sockaddr_in peer_addr;
    struct thread_info *info;
    FILE *f;

    if (argc < 2) {
        printf("Usage: %s [-h host]|[-p port] filename\n"
               "host - ipv4 address\n"
               "port - number of port to be listening\n"
               "filename - name of file that acts as a database\n", argv[0]);
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
            printf("Usage: %s [-h host]|[-p port] filename\n"
                   "host - ipv4 address\n"
                   "port - number of port to be listening\n"
                   "filename - name of file that acts as a database\n", argv[0]);
            exit(EXIT_FAILURE);
        }
    }

    f = get_file(argv[optind]);
    if (f == NULL) {
        exit_with_error("Could not find/create file");
    }

    sock_fd = get_socket(host, port);

    if (listen(sock_fd, BACKLOG) == -1) {
        exit_with_error("could not listen to a port");
    }

    peer_len = sizeof(struct sockaddr_in);
    info_size = sizeof(struct thread_info);

    while (1) {
        conn_fd = accept(sock_fd, (struct sockaddr*) &peer_addr, &peer_len);
        if (conn_fd == -1) {
            exit_with_error("could not establish connection");
        }
        log("connection established");


        info = malloc(info_size);
        info->conn_fd = conn_fd;
        info->peer_addr = &peer_addr;
        info ->peer_len = &peer_len;
        info->f = f;

        if (pthread_create(&info->t_id, NULL, &main_routine, info)) {
            free(info);
            exit_with_error("failed to create a new thread");
        }
        log("created new thread");
    }
}
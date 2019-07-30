#include "../headers/connection.h"

int connect_to_server(const char *host, const char *port) {
    int sock_fd, status;
    struct addrinfo hints;
    struct addrinfo *addr_list, *ai;

    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_protocol = 0;
    hints.ai_flags = 0;
    hints.ai_addr = NULL;
    hints.ai_next = NULL;
    hints.ai_canonname = NULL;

    status = getaddrinfo(host, port, &hints, &addr_list);
    if (status) {
        return -1;
    }

    for (ai = addr_list; ai != NULL; ai = ai->ai_next) {
        sock_fd = socket(ai->ai_family, ai->ai_socktype, ai->ai_protocol);

        if (sock_fd == -1)
            continue;

        if (!connect(sock_fd, ai->ai_addr, ai->ai_addrlen))
            break;

        close(sock_fd);
    }

    if (ai == NULL) {
        return -1;
    }

    freeaddrinfo(addr_list);

    return sock_fd;
}

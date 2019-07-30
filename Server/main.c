#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <stdlib.h>
#include <arpa/inet.h>
#include <netdb.h>

#define BACKLOG 10
#define SIZE 100

void exit_with_error(const char* message) {
    printf("Error: %s", message);
    exit(EXIT_FAILURE);
}

int main(int argc, char** argv)
{
    int sock_fd, conn_fd, status;
    char BUF[SIZE];
    socklen_t peer_len;
    struct addrinfo hints;
    struct addrinfo *addr_list, *ai;
    struct sockaddr_in peer_addr;

    if (argc < 3)
        exit_with_error("ip address or port missing");

    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_protocol = 0;
    hints.ai_flags = 0;
    hints.ai_canonname = NULL;
    hints.ai_addr = NULL;
    hints.ai_next = NULL;

    status = getaddrinfo(argv[1], argv[2], &hints, &addr_list);
    if (status)
        exit_with_error("could not get a socket");

    for (ai = addr_list; ai != NULL; ai = ai->ai_next) {
        sock_fd = socket(ai->ai_family, ai->ai_socktype, ai->ai_protocol);

        if (sock_fd==-1)
            continue;

        if (bind(sock_fd, ai->ai_addr, ai->ai_addrlen) == 0)
            break;

        close(sock_fd);
    }

    if (ai == NULL)
        exit_with_error("bind failed");

    freeaddrinfo(addr_list);

    if (listen(sock_fd, BACKLOG) == -1)
        exit_with_error("could not listen to a port");

    peer_len = sizeof(struct sockaddr_in);
    conn_fd = accept(sock_fd, (struct sockaddr*) &peer_addr, &peer_len);
    if (conn_fd == -1)
        exit_with_error("could not establish connection");

    while (1) {
        if (recvfrom(conn_fd, BUF, SIZE, 0, (struct sockaddr*) &peer_addr, &peer_len) > 0) {
            sendto(conn_fd, "Accepted", SIZE, 0, (const struct sockaddr*) &peer_addr, peer_len);
            break;
        }
    }

    exit(EXIT_SUCCESS);
}
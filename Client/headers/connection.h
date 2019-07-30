#ifndef CLIENT_CONNECTION_H
#define CLIENT_CONNECTION_H

#include <netdb.h>
#include <unistd.h>

int connect_to_server(const char *host, const char *port);

#endif //CLIENT_CONNECTION_H

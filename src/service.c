#include "service.h"

int init_server(Server *server, int port)
{
    int opt = 1;
    server->serverSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (server->serverSocket < 0)
        handle_error("socket", -1);

    server->serverAddr.sin_family = AF_INET;
    server->serverAddr.sin_port = htons(port);
    server->serverAddr.sin_addr.s_addr = htonl(INADDR_ANY);

    if (setsockopt(server->serverSocket, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)))
        handle_error("setsockopt", -1);

    if (bind(server->serverSocket, (struct sockaddr *)&server->serverAddr, sizeof(server->serverAddr)) < 0)
        handle_error("bind", -1);

    return 0;
}

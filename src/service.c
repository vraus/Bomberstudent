#include "../include/service.h"

int init_server(Server *server, int port, int type)
{
    int opt = 1;
    server->server_socket = socket(AF_INET, type, 0);
    if (server->server_socket < 0)
        handle_error("socket", -1);

    server->server_addr.sin_family = AF_INET;
    server->server_addr.sin_port = htons(port);
    server->server_addr.sin_addr.s_addr = htonl(INADDR_ANY);

    if (setsockopt(server->server_socket, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)))
        handle_error("setsockopt", -1);

    if (bind(server->server_socket, (struct sockaddr *)&server->server_addr, sizeof(server->server_addr)) < 0)
        handle_error("bind", -1);

    return 0;
}

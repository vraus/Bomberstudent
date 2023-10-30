#include "service.h"
#include "allocationsManager.h"

Server *serverManager = NULL;
char *msg = NULL;
int port = 0;

void kill_handler(int n)
{
    free_all_allocations();
    exit(EXIT_SUCCESS);
}

int old()
{
    signal(SIGINT, kill_handler);
    int num_servers = 1;
    serverManager = (Server *)malloc(num_servers * sizeof(Server));

    port = 42069;
    if (init_server(&serverManager[0], 42069, "t") < 0)
        handle_error("init_server", -1);

    if (add_server(&serverManager[0]) < 0)
        handle_error("add_server", -1);

    fd_set activeSockets;
    FD_ZERO(&activeSockets);

    for (int i = 0; i < num_servers; i++)
        FD_SET(serverManager[i].serverSocket, &activeSockets);

    for (;;)
    {
        fd_set readSockets = activeSockets;
        if (select(FD_SETSIZE, &readSockets, NULL, NULL, NULL) < 0)
            handle_error("select", -1);

        for (int i = 0; i < num_servers; i++)
        {
            if (FD_ISSET(serverManager[i].serverSocket, &readSockets))
            {
                struct sockaddr_in clientAddr;
                socklen_t clientSize = sizeof(clientAddr);

                int client = accept(serverManager[i].serverSocket, (struct sockaddr *)&clientAddr, &clientSize);
                if (client < 0)
                    handle_error("accept", -1);
                else
                    printf("client accepted on server %d\n", serverManager[i].serverSocket);

                msg = "Connecté au server\n";
                if (send(client, msg, strlen(msg), 0) < 0)
                    handle_error("send", -1);
            }
        }
    }
    return 0;
}
#include "service.h"
#include "allocationsManager.h"

#define MAX_SERVERS 200
#define MAX_CLIENTS 100
#define GAME_LIST_PATH "json/gameslist.json"
#define MAPS_LIST_PATH "json/mapslist.json"

int port = 0;
int num_clients = 0;
char buffer[1024];

void kill_handler(int n)
{
    free_all_allocations();
    exit(EXIT_SUCCESS);
}

int actionGameCreate(int *cFd)
{
    FILE *gamesListFile = fopen(GAME_LIST_PATH, "r++");
    if (!gamesListFile)
        handle_error("fopen", -1);

    if (add_file(gamesListFile) < 0)
        handle_error("add_file", -1);

    fseek(gamesListFile, 0, SEEK_END);
    long tell = ftell(gamesListFile);
    fseek(gamesListFile, 0, SEEK_SET);

    char *content = (char *)malloc(tell + 1);
    if (!content)
        handle_error("content: malloc", -1);

    if (add_string(content) < 0)
        handle_error("content: add_string", -1);

    fread(content, 1, tell, gamesListFile);
    content[tell] = '\0';

    printf("\n%s\n", content);

    // TODO: Create a JSON Object and populate it with data
    json_t *root;
    json_error_t error;
    root = json_loads(content, 0, &error);

    if (!root)
    {
        free(content);
        handle_error("json_loads", -1);
    }

    if (add_json_t(root) < 0)
        handle_error("add_json_t", -1);

    // TODO: Add key-value pairs to the JSON object

    // TODO: Write the JSON object to the file

    return 0;
}

int actionGameList(int *cFd)
{
    FILE *gamesListFile = fopen(GAME_LIST_PATH, "r");
    if (!gamesListFile)
        handle_error("fopen", -1);

    if (add_file(gamesListFile) < 0)
        handle_error("add_file", -1);

    fseek(gamesListFile, 0, SEEK_END);
    long tell = ftell(gamesListFile);
    fseek(gamesListFile, 0, SEEK_SET);

    char *content = (char *)malloc(tell + 1);
    if (!content)
        handle_error("content: malloc", -1);

    if (add_string(content) < 0)
        handle_error("add_string", -1);

    fread(content, 1, tell, gamesListFile);
    content[tell] = '\0';

    json_t *root;
    json_error_t error;
    root = json_loads(content, 0, &error);

    if (!root)
    {
        free(content);
        handle_error("json_loads", -1);
    }

    if (add_json_t(root) < 0)
        handle_error("add_json_t", -1);

    send(*cFd, content, strlen(content), 0);

    free(content);
    fclose(gamesListFile);
    return 0;
}

int actionMapsList(int *cFd)
{
    FILE *mapsListFile = fopen(MAPS_LIST_PATH, "r");
    if (!mapsListFile)
        handle_error("fopen", -1);

    if (add_file(mapsListFile) < 0)
        handle_error("add_file", -1);

    fseek(mapsListFile, 0, SEEK_END);
    long tell = ftell(mapsListFile);
    fseek(mapsListFile, 0, SEEK_SET);

    char *content = (char *)malloc(tell + 1);
    if (!content)
        handle_error("malloc", -1);

    if (add_string(content) < 0)
        handle_error("add_string", -1);

    fread(content, 1, tell, mapsListFile);
    content[tell] = '\0';

    json_t *root;
    json_error_t error;
    root = json_loads(content, 0, &error);

    if (!root)
    {
        free(content);
        handle_error("json_loads", -1);
    }

    if (add_json_t(root) < 0)
        handle_error("add_json_t", -1);

    send(*cFd, content, strlen(content), 0);

    free(content);
    fclose(mapsListFile);
    return 0;
}

int setupServerManager(Server *serverManager)
{
    if (init_server(serverManager, 42069) < 0)
        handle_error("init_server", -1);

    if (add_server(serverManager) < 0)
        handle_error("add_server", -1);

    if (listen(serverManager->serverSocket, MAX_CLIENTS) < 0)
        handle_error("serverManager: listen", -1);

    printf("I'm listening...\n");

    return 0;
}

int main(int argc, char *argv[])
{
    signal(SIGINT, kill_handler);
    Server serverManager;
    port = 42069;

    int client_sockets[MAX_CLIENTS];
    int new_socket;
    struct sockaddr_in cAddr;
    socklen_t cAddr_len = sizeof(cAddr);

    fd_set read_fds;

    if (add_string(buffer) < 0)
        handle_error("buffer: addstring", -1);

    int activity, index, valread;

    for (index = 0; index < MAX_CLIENTS; index++)
        client_sockets[index] = 0;

    if (setupServerManager(&serverManager) < 0)
        handle_error("serverManager: setupServerManager", -1);

    for (;;)
    {
        FD_ZERO(&read_fds);
        FD_SET(serverManager.serverSocket, &read_fds);
        int max_sd = serverManager.serverSocket;

        for (index = 0; index < MAX_CLIENTS; index++) // Add clients to the read_fds
        {
            int sd = client_sockets[index];

            if (sd > 0)
                FD_SET(sd, &read_fds);

            if (sd > max_sd)
                max_sd = sd;
        }

        // Await client activity
        activity = select(max_sd + 1, &read_fds, NULL, NULL, NULL);
        if (activity < 0)
            handle_error("activity: select", -1);

        // Accept all new connexions detected
        if (FD_ISSET(serverManager.serverSocket, &read_fds))
        {
            if ((new_socket = accept(serverManager.serverSocket, (struct sockaddr *)&cAddr, (socklen_t *)&cAddr_len)) < 0)
                handle_error("new_socket: accept", -1);

            // Add the new client to the client_sockets group
            for (index = 0; index < MAX_CLIENTS; index++)
            {
                if (client_sockets[index] == 0)
                {
                    num_clients++;
                    client_sockets[index] = new_socket;
                    printf("New connection, socket fd is %d\n", new_socket);
                    break;
                }
            }
        }

        // Check for incoming messages
        for (index = 0; index < MAX_CLIENTS; index++)
        {
            int sd = client_sockets[index];

            if (FD_ISSET(sd, &read_fds))
            {
                if ((valread = read(sd, buffer, sizeof(buffer))) == 0)
                {
                    // Disconnects the client
                    getpeername(sd, (struct sockaddr *)&cAddr, (socklen_t *)&cAddr_len);
                    printf("Host Disconnected, ip %s, port %d\n", inet_ntoa(cAddr.sin_addr), ntohs(cAddr.sin_port));
                    close(sd);
                    client_sockets[index] = 0;
                    num_clients--;
                }
                else
                {
                    char response[256];
                    buffer[valread] = '\0';
                    // TODO: When received creates a UDP server in a pthread and connects the client fd to it
                    if (strncmp(buffer, "POST game/create", 16) == 0)
                    {
                        if (actionGameCreate(&client_sockets[index]) < 0)
                            handle_error_noexit("POST game/create");
                    }
                    else if (strncmp(buffer, "GET maps/list", 13) == 0)
                    {
                        if (actionMapsList(&client_sockets[index]) < 0)
                            handle_error_noexit("GET maps/list");
                    }
                    else if (strncmp(buffer, "GET game/list", 13) == 0)
                    {
                        if (actionGameList(&client_sockets[index]) < 0)
                            handle_error_noexit("GET game/list");
                    }
                    else if (strncmp(buffer, "looking for bomberstudent servers", 33) == 0)
                        sprintf(response, "hello i'm a bomberstudent server.\n");
                    else
                    {
                        sprintf(response, "Unkowned command.\n"
                                          "List of commands:\n"
                                          " - 'looking for bomberstudent servers'\n"
                                          " - 'GET maps/list'\n"
                                          " - 'GET game/list'\n"
                                          " - 'POST game/create'\n");
                    }
                    response[strlen(response) + 1] = '\0';
                    if (response != NULL)
                        send(client_sockets[index], response, strlen(response), 0);
                }
            }
        }
    }

    return 0;
}
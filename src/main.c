#include "service.h"
#include "allocationsManager.h"

#define MAX_SERVERS 200
#define MAX_CLIENTS 200
#define GAME_LIST_PATH "json/gameslist.json"
#define MAPS_LIST_PATH "json/mapslist.json"
#define GAME_CREATE_PATH "json/gamecreate.json"

int port = 0;
int num_clients = 0;
char buffer[1024];

/** @brief Signal handler to free allocations when SIGINT on the main server */
void kill_handler(int n)
{
    free_all_allocations();
    exit(EXIT_SUCCESS);
}

int readJsonFile(int *cFd, char *path_json_file, char **content)
{
    FILE *games_list_file = fopen(path_json_file, "r++");
    if (!games_list_file)
        handle_error("fopen", -1);

    fseek(games_list_file, 0, SEEK_END);
    long tell = ftell(games_list_file);
    fseek(games_list_file, 0, SEEK_SET);

    *content = (char *)calloc(tell, sizeof(char));
    if (fread(*content, 1, tell, games_list_file) < 0)
        handle_error("freadJsonFile", -1);

    content[0][tell] = '\0';

    fclose(games_list_file);
    return 0;
}

/** @brief Function called when client send `POST game/create` request.
 *  @return -1 when in error case (using the `handle_error` MACRO). 0 when no errors
 */
int actionGameCreate(int *cFd)
{
    // TODO: Voir si la méthode peut rentrer dans une fonction
    FILE *game_create_json = fopen(GAME_LIST_PATH, "r");
    if (game_create_json == NULL)
        handle_error("fopen gameCreateJson r", -1);

    fseek(game_create_json, 0, SEEK_END);
    long tell = ftell(game_create_json);
    fseek(game_create_json, 0, SEEK_SET);

    char *content = (char *)calloc(tell, sizeof(char));
    if (fread(content, 1, tell, game_create_json) < 0)
        handle_error("fread acionGameCreate", -1);

    fclose(game_create_json);

    // Parse the actual gamesList json file
    cJSON *root = cJSON_Parse(content);
    if (root == NULL)
    {
        const char *error_ptr = cJSON_GetErrorPtr();
        if (error_ptr != NULL)
            handle_error_noexit("root: Error");
        cJSON_Delete(root);
        return 1;
    }

    char buff[strlen(buffer) - 17];
    for (int i = 0; i < strlen(buffer) - 17; i++)
        buff[i] = buffer[i + 17];

    buff[strlen(buff) - 1] = '\0';

    // Parse the client request of creating new game
    cJSON *new_game = cJSON_Parse(buff);
    if (new_game == NULL)
    {
        const char *error_ptr = cJSON_GetErrorPtr();
        if (error_ptr != NULL)
            handle_error_noexit("new_game: Error");
        cJSON_Delete(new_game);
        return 1;
    }

    // Indents nbGameList
    cJSON *nb_games_list = cJSON_GetObjectItemCaseSensitive(root, "nbGamesList");
    cJSON_ReplaceItemInObjectCaseSensitive(root, "nbGamesList", cJSON_CreateNumber(nb_games_list->valueint + 1));

    cJSON *array_games = cJSON_GetObjectItemCaseSensitive(root, "games");
    cJSON *new_game_data = cJSON_CreateObject();
    cJSON *new_game_name = cJSON_GetObjectItemCaseSensitive(new_game, "name");
    cJSON *new_game_map = cJSON_GetObjectItemCaseSensitive(new_game, "mapId");
    cJSON_AddStringToObject(new_game_data, "name", new_game_name->valuestring);
    cJSON_AddNumberToObject(new_game_data, "nbPlayers", 1);
    cJSON_AddNumberToObject(new_game_data, "mapId", new_game_map->valueint);
    cJSON_AddItemToArray(array_games, new_game_data);

    char *json_str = cJSON_Print(root);
    game_create_json = fopen(GAME_LIST_PATH, "w");

    if (game_create_json == NULL)
        handle_error("fopen gameCreateJson w", -1);

    printf("%s\n", json_str);
    fputs(json_str, game_create_json);
    fclose(game_create_json);

    cJSON_Delete(root);
    cJSON_Delete(new_game);

    return 0;
}

/**
 * @brief Function used to setup the main Server. This will instantiate
 * using `service.h` functions to bind the socket.
 * Then `setupServerManager()` put the given server into listening mode.
 * @param server_manager `struct Server` pointing to the server to setup.
 * @return -1 when in error case (using the `handle_error` MACRO). 0 when no errors
 */
int setupServerManager(Server *server_manager)
{
    if (init_server(server_manager, 42069) < 0)
        handle_error("init_server", -1);

    if (add_server(server_manager) < 0)
        handle_error("add_server", -1);

    if (listen(server_manager->server_socket, MAX_CLIENTS) < 0)
        handle_error("server_manager: listen", -1);

    printf("I'm listening...\n");

    return 0;
}

/**
 * @brief Function to process the received message from a client
 * @param client_socket FD of the client
 * @param buffer message sent by the client
 */
int answerServer(int client_socket, char *buffer)
{
    char *response = (char *)calloc(256, sizeof(char));

    if (strncmp(buffer, "POST game/create", 16) == 0)
    {
        if (actionGameCreate(&client_socket) < 0)
            handle_error_noexit("POST game/create");
    }
    else if (strncmp(buffer, "GET maps/list", 13) == 0)
    {
        if (readJsonFile(&client_socket, MAPS_LIST_PATH, &response) < 0)
            handle_error_noexit("GET maps/list");
    }
    else if (strncmp(buffer, "GET game/list", 13) == 0)
    {
        if (readJsonFile(&client_socket, GAME_LIST_PATH, &response) < 0)
            handle_error_noexit("GET game/list");
    }
    else if (strncmp(buffer, "looking for bomberstudent servers", 33) == 0)
    {

        sprintf(response, "hello i'm a bomberstudent server.\n");
        response[strlen(response) + 1] = '\0';
    }
    else
    {
        sprintf(response, "Unkowned command.\n"
                          "List of commands:\n"
                          " - 'looking for bomberstudent servers'\n"
                          " - 'GET maps/list'\n"
                          " - 'GET game/list'\n"
                          " - 'POST game/create'\n");
        response[strlen(response) + 1] = '\0';
    }

    if (response != NULL)
        send(client_socket, response, strlen(response), 0);

    return 0;
}

// TODO: multithreader le server
//  Enlever du gras
int main(int argc, char *argv[])
{
    signal(SIGINT, kill_handler);
    Server server_manager;
    port = 42069;

    int client_sockets[MAX_CLIENTS];
    int new_socket;
    struct sockaddr_in c_addr;
    socklen_t c_addr_len = sizeof(c_addr);

    fd_set read_fds;

    if (add_string(buffer) < 0)
        handle_error("buffer: addstring", -1);

    int activity, index, valread;

    for (index = 0; index < MAX_CLIENTS; index++)
        client_sockets[index] = 0;

    if (setupServerManager(&server_manager) < 0)
        handle_error("server_manager: setupServerManager", -1);

    for (;;)
    {
        FD_ZERO(&read_fds);
        FD_SET(server_manager.server_socket, &read_fds);
        int max_sd = server_manager.server_socket;

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
        if (FD_ISSET(server_manager.server_socket, &read_fds))
        {
            if ((new_socket = accept(server_manager.server_socket, (struct sockaddr *)&c_addr, (socklen_t *)&c_addr_len)) < 0)
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
                    getpeername(sd, (struct sockaddr *)&c_addr, (socklen_t *)&c_addr_len);
                    printf("Host Disconnected, ip %s, port %d\n", inet_ntoa(c_addr.sin_addr), ntohs(c_addr.sin_port));
                    close(sd);
                    client_sockets[index] = 0;
                    num_clients--;
                }
                else
                {
                    buffer[valread] = '\0';
                    if (answerServer(client_sockets[index], buffer) < 0)
                        handle_error_noexit("answerServer");
                }
            }
        }
    }

    return 0;
}
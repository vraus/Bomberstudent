#include "service.h"
#include "allocationsManager.h"

#include <pthread.h>

#define handle_json_error(msg)                       \
    do                                               \
    {                                                \
        const char *error_ptr = cJSON_GetErrorPtr(); \
        if (error_ptr != NULL)                       \
            handle_error_noexit("msg");              \
    } while (0)

#define MAX_CLIENTS 200 // Max Number of pthread for clients
#define MAX_SIZE_MESSAGE 1024
#define GAME_LIST_PATH "json/gameslist.json"
#define MAPS_LIST_PATH "json/mapslist.json"
#define GAME_CREATE_PATH "json/gamecreate.json"
#define GAME_JOIN_PATH "json/gamejoin.json"

int port = 42069;
int num_clients = 0;

/** @brief Signal handler to free allocations when SIGINT on the main server */
void kill_handler(int n)
{
    free_all_allocations();
    exit(EXIT_SUCCESS);
}

/**
 * @brief Function used to setup the main Server. This will instantiate
 * using `service.h` functions to bind the socket.
 * Then `setup_server_socket()` put the given server into listening mode.
 * @param server_manager `struct Server` pointing to the server to setup.
 * @return -1 when in error case (using the `handle_error` MACRO). 0 when no errors
 */
int setup_server_socket(Server *server_manager, int port, int type)
{
    if (init_server(server_manager, port, type) < 0)
        handle_error("init_server", -1);

    if (add_server(server_manager) < 0)
        handle_error("add_server", -1);

    if (listen(server_manager->server_socket, MAX_CLIENTS) < 0)
        handle_error("server_manager: listen", -1);

    printf("I'm listening...\n");

    return 0;
}

/** @brief Reads a json file and sends it's content to the client file descriptor */
int read_json_file(char *path_json_file, char **content)
{
    FILE *games_list_file = fopen(path_json_file, "r");
    if (!games_list_file)
        handle_error("fopen games_list_file \"r\"", -1);

    fseek(games_list_file, 0, SEEK_END);
    long tell = ftell(games_list_file);
    fseek(games_list_file, 0, SEEK_SET);

    *content = (char *)calloc(tell, sizeof(char));
    if (fread(*content, 1, tell, games_list_file) < 0)
        handle_error("read_json_file(): fread", -1);

    content[0][tell] = '\0';

    fclose(games_list_file);
    return 0;
}

/** @brief Function to update the game file when a client connects to the game he chose. */
//XXX: récupérer la bonne mapId dans gamelist pour mettre a jour gamejoin
int action_game_join(char *buffer)
{
    char buff[strlen(buffer) - 15];
    for (int i = 0; i < strlen(buffer) - 15; i++)
        buff[i] = buffer[i + 15];

    buff[strlen(buff) - 1] = '\0';

    cJSON *file_join_game = cJSON_Parse(buff);
    if (file_join_game == NULL)
    {
        handle_json_error("file_join_game");
        cJSON_Delete(file_join_game);
        return -1;
    }

    cJSON *game_name = cJSON_GetObjectItemCaseSensitive(file_join_game, "name");

    char *content = (char *)calloc(MAX_SIZE_MESSAGE, sizeof(char));
    read_json_file(GAME_LIST_PATH, &content);

    cJSON *cj_games_list = cJSON_Parse(content);
    if (cj_games_list == NULL)
    {
        handle_json_error("cj_games_list");
        cJSON_Delete(cj_games_list);
        free(content);
        return -1;
    } else if (cj_games_list != NULL) {
        cJSON *games = cJSON_GetObjectItemCaseSensitive(cj_games_list, "games");
        if (games != NULL)
        {
            cJSON *element;
            cJSON_ArrayForEach(element, games)
            {
                cJSON *element_name = cJSON_GetObjectItemCaseSensitive(element, "name");
                if (strcmp(element_name->valuestring, game_name->valuestring) == 0)
                {
                    cJSON *element_nb_player = cJSON_GetObjectItemCaseSensitive(element, "nbPlayers");
                    cJSON_ReplaceItemInObjectCaseSensitive(element, "nbPlayers", cJSON_CreateNumber(element_nb_player->valueint + 1));
                    char *json_print = cJSON_Print(cj_games_list);
                    FILE *games_list = fopen(GAME_LIST_PATH, "w");
                    if (games_list == NULL)
                         handle_error("fopen game_list \"w\"", -1);

                    printf("%s\n", json_print);
                    fputs(json_print, games_list);
                    fclose(games_list);
                    break;
                }
            }
        }
    }

    cJSON_Delete(cj_games_list);
    free(content);
    cJSON_Delete(file_join_game);

    return 0;
}


/** @brief Function called when client send `POST game/create` request.
 *  modify the gamelist.json
 *  @return -1 when in error case (using the `handle_error` MACRO). 0 when no errors
 */
int action_game_create(char *buffer)
{
    char *content = (char *)calloc(MAX_SIZE_MESSAGE, sizeof(char));
    read_json_file(GAME_LIST_PATH, &content);

    // Parse the actual gamesList json file
    cJSON *root = cJSON_Parse(content);
    if (root == NULL)
    {
        handle_json_error("root");
        cJSON_Delete(root);
        return -1;
    }

    char buff[strlen(buffer) - 17];
    for (int i = 0; i < strlen(buffer) - 17; i++)
        buff[i] = buffer[i + 17];

    buff[strlen(buff) - 1] = '\0';

    // Parse the client request of creating new game
    cJSON *new_game = cJSON_Parse(buff);
    if (new_game == NULL)
    {
        handle_json_error("new_game");
        cJSON_Delete(new_game);
        return -1;
    }

    // Indents nbGameList
    cJSON *nb_games_list = cJSON_GetObjectItemCaseSensitive(root, "nbGamesList");
    cJSON_ReplaceItemInObjectCaseSensitive(root, "nbGamesList", cJSON_CreateNumber(nb_games_list->valueint + 1));

    cJSON *array_games = cJSON_GetObjectItemCaseSensitive(root, "games");
    cJSON *new_game_data = cJSON_CreateObject();
    cJSON *new_game_name = cJSON_GetObjectItemCaseSensitive(new_game, "name");
    cJSON *new_game_map = cJSON_GetObjectItemCaseSensitive(new_game, "mapId");
    if (cJSON_IsString(new_game_name) && (new_game_name->valuestring != NULL))
        cJSON_AddStringToObject(new_game_data, "name", new_game_name->valuestring);
    cJSON_AddNumberToObject(new_game_data, "nbPlayers", 1);
    if (cJSON_IsNumber(new_game_map))
        cJSON_AddNumberToObject(new_game_data, "mapId", new_game_map->valueint);
    cJSON_AddItemToArray(array_games, new_game_data);

    char *json_str = cJSON_Print(root);
    FILE *game_create_json = fopen(GAME_LIST_PATH, "w");

    if (game_create_json == NULL)
        handle_error("fopen game_create_json \"w\"", -1);

    printf("%s\n", json_str);
    fputs(json_str, game_create_json);
    fclose(game_create_json);

    cJSON_Delete(root);
    cJSON_Delete(new_game);

    return 0;
}

/**
 * @brief Function where the thread handles the client requests
 * @param client_socket FD of the client
 * @param buffer message sent by the client
 */
void *answer_server(void *arg)
{
    int client_socket = *((int *)arg), id = num_clients;
    ssize_t rd;
    char *response = (char *)calloc(256, sizeof(char));
    char *buffer = (char *)calloc(MAX_SIZE_MESSAGE, sizeof(char));

    while ((rd = read(client_socket, buffer, MAX_SIZE_MESSAGE)) > 0)
    {
        buffer[rd] = '\0';
        printf("User %d: %s\n", id, buffer);
        if (strncmp(buffer, "POST game/create", 16) == 0)
        {
            action_game_create(buffer);
            if (read_json_file(GAME_CREATE_PATH, &response) < 0)
                handle_error_noexit("GET game/create");
        }
        else if (strncmp(buffer, "POST game/join", 14) == 0)
        {
            action_game_join(buffer);
            if (read_json_file(GAME_JOIN_PATH, &response) < 0)
                handle_error_noexit("GET game/join");
        }
        else if (strncmp(buffer, "GET maps/list", 13) == 0)
        {
            if (read_json_file(MAPS_LIST_PATH, &response) < 0)
                handle_error_noexit("GET maps/list");
        }
        else if (strncmp(buffer, "GET game/list", 13) == 0)
        {
            if (read_json_file(GAME_LIST_PATH, &response) < 0)
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
                              " - 'POST game/create'\n"
                              " - 'POST game/join'\n");
            response[strlen(response) + 1] = '\0';
        }

        if (response != NULL)
            send(client_socket, response, strlen(response), 0);
    }
    printf("Client %d disconnected.\n", id);
    close(client_socket);
    pthread_exit(NULL);
}

int main(int argc, char *argv[])
{
    signal(SIGINT, kill_handler);
    Server server_manager;

    pthread_t threads[MAX_CLIENTS];

    int client_sockets[MAX_CLIENTS];
    int index = 0;
    struct sockaddr_in c_addr;
    socklen_t c_addr_len = sizeof(c_addr);

    if (setup_server_socket(&server_manager, 42069, SOCK_STREAM) < 0)
        handle_error("server_manager: setup_server_socket", -1);

    for (;;)
    {
        if ((client_sockets[index] = accept(server_manager.server_socket, (struct sockaddr *)&c_addr, &c_addr_len)) < 0)
            handle_error("accept", -1);

        printf("New user joined: %d\n", ++num_clients);

        if (pthread_create(&threads[index], NULL, answer_server, (void *)&client_sockets[index]) != 0)
            handle_error("pthread_create", -1);

        index++;
    }

    return 0;
}
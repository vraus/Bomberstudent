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
int read_json_file(int *cFd, char *path_json_file, char **content)
{
    FILE *games_list_file = fopen(path_json_file, "r++");
    if (!games_list_file)
        handle_error("fopen games_list_file \"r++\"", -1);

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

int action_game_join(int *cFd, char *buffer)
{
    char buff[strlen(buffer) - 15];
    for (int i = 0; i < strlen(buffer) - 15; i++)
        buff[i] = buffer[i + 15];

    buff[strlen(buff) - 1] = '\0';

    cJSON *file_join_game = cJSON_Parse(buff);
    if (file_join_game == NULL)
    {
        handle_json_error("new_game");
        cJSON_Delete(file_join_game);
        return -1;
    }

    cJSON *game_name = cJSON_GetObjectItemCaseSensitive(file_join_game, "name");
    if (!(cJSON_IsString(game_name)) || (game_name->valuestring == NULL))
        handle_error("game_name: bad usage", -1);

    const char *join_game_file = "json/";
    size_t len_path = strlen(join_game_file) + strlen(game_name->valuestring) + 1;
    char join_game_path[len_path];
    sprintf(join_game_path, "%s%s.json", join_game_file, game_name->valuestring);

    FILE *game = fopen(join_game_path, "r");
    if (game == NULL)
        handle_error("attempt to join game failed", -1);

    fseek(game, 0, SEEK_END);
    long tell = ftell(game);
    fseek(game, 0, SEEK_SET);

    char *content = (char *)calloc(tell, sizeof(char));
    if (fread(content, 1, tell, game) < 0)
        handle_error("action_game_create(): fread", -1);

    fclose(game);

    cJSON *root = cJSON_Parse(content);
    if (root == NULL)
    {
        handle_json_error("root");
        cJSON_Delete(root);
        return -1;
    }

    cJSON *nbPlayers = cJSON_GetObjectItemCaseSensitive(root, "nbPlayers");
    cJSON_ReplaceItemInObjectCaseSensitive(root, "nbPlayers", cJSON_CreateNumber(nbPlayers->valueint + 1));

    cJSON *array_players = cJSON_GetObjectItemCaseSensitive(root, "players");
    cJSON *join_player_data = cJSON_CreateObject();
    cJSON_AddNumberToObject(join_player_data, "id", *cFd);
    cJSON *join_player_pos = cJSON_GetObjectItemCaseSensitive(root, "startPos");
    cJSON_AddStringToObject(join_player_data, "pos", join_player_pos->valuestring);
    cJSON_ReplaceItemInObjectCaseSensitive(root, "startPos", cJSON_CreateString("3,2"));
    cJSON_AddItemToArray(array_players, join_player_data);

    char *json_str = cJSON_Print(root);
    game = fopen(join_game_path, "w");
    if (game == NULL)
        handle_error("fopen game_create_json \"w\"", -1);

    printf("%s\n", json_str);
    fputs(json_str, game);
    fclose(game);

    cJSON_Delete(root);
    cJSON_Delete(file_join_game);

    return 0;
}

int create_running_game_data(char *game_name, int *cFd)
{
    const char *new_game_file = "json/";
    size_t len_path = strlen(new_game_file) + strlen(game_name) + 1;
    char new_game_path[len_path];
    sprintf(new_game_path, "%s%s.json", new_game_file, game_name);

    FILE *game = fopen(new_game_path, "w");
    if (game == NULL)
        handle_error("temp game file fopen", -1);

    fclose(game);

    cJSON *new_game = cJSON_CreateObject();
    cJSON_AddStringToObject(new_game, "action", "game/join");
    cJSON_AddStringToObject(new_game, "statut", "201");
    cJSON_AddStringToObject(new_game, "message", "game joined");
    cJSON_AddNumberToObject(new_game, "nbPlayers", 1);
    cJSON *players = cJSON_CreateArray();
    cJSON *player_info = cJSON_CreateObject();
    cJSON_AddNumberToObject(player_info, "id", *cFd);
    cJSON_AddStringToObject(player_info, "pos", "5,3");
    cJSON_AddItemToArray(players, player_info);
    cJSON_AddItemToObject(new_game, "players", players);
    cJSON_AddStringToObject(new_game, "startPos", "5,3");
    cJSON *player = cJSON_CreateObject();
    cJSON_AddNumberToObject(player, "life", 100);
    cJSON_AddNumberToObject(player, "speed", 1);
    cJSON_AddNumberToObject(player, "nbClassicBomb", 1);
    cJSON_AddNumberToObject(player, "nbMine", 0);
    cJSON_AddNumberToObject(player, "nbRemoteBomb", 0);
    cJSON_AddNumberToObject(player, "impactDist", 2);
    cJSON_AddBoolToObject(player, "invincible", cJSON_False);
    cJSON_AddItemToObject(new_game, "player", player);

    char *json_print = cJSON_Print(new_game);

    game = fopen(new_game_path, "w");
    if (game == NULL)
        handle_error("temp game file fopen", -1);

    fputs(json_print, game);
    fclose(game);
    return 0;
}

int running_game(int *cFd, char *game_name)
{
    const char *new_game_file = "json/";
    size_t len_path = strlen(new_game_file) + strlen(game_name) + 1;
    char game_path[len_path];
    sprintf(game_path, "%s%s.json", new_game_file, game_name);

    char *response = (char *)calloc(256, sizeof(char));
    if (read_json_file(cFd, game_path, &response) < 0)
        handle_error("running_game(): read_json_file()", -1);

    if (response != NULL)
        send(*cFd, response, strlen(response), 0);

    return 0;
}

/** @brief Function called when client send `POST game/create` request.
 *  @return -1 when in error case (using the `handle_error` MACRO). 0 when no errors
 */
int action_game_create(int *cFd, char *buffer)
{
    FILE *game_create_json = fopen(GAME_LIST_PATH, "r");
    if (game_create_json == NULL)
        handle_error("fopen game_create_json \"r\"", -1);

    fseek(game_create_json, 0, SEEK_END);
    long tell = ftell(game_create_json);
    fseek(game_create_json, 0, SEEK_SET);

    char *content = (char *)calloc(tell, sizeof(char));
    if (fread(content, 1, tell, game_create_json) < 0)
        handle_error("action_game_create(): fread", -1);

    fclose(game_create_json);

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

    if (create_running_game_data(new_game_name->valuestring, cFd) < 0)
        handle_error("action_game_create(): create_running_game_data()", -1);

    char *json_str = cJSON_Print(root);
    game_create_json = fopen(GAME_LIST_PATH, "w");

    if (game_create_json == NULL)
        handle_error("fopen game_create_json \"w\"", -1);

    printf("%s\n", json_str);
    fputs(json_str, game_create_json);
    fclose(game_create_json);

    cJSON_Delete(root);
    running_game(cFd, new_game_name->valuestring);
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
            action_game_create(&client_socket, buffer);
        }
        else if (strncmp(buffer, "POST game/join", 14) == 0)
        {
            action_game_join(&client_socket, buffer);
        }
        else if (strncmp(buffer, "GET maps/list", 13) == 0)
        {
            if (read_json_file(&client_socket, MAPS_LIST_PATH, &response) < 0)
                handle_error_noexit("GET maps/list");
        }
        else if (strncmp(buffer, "GET game/list", 13) == 0)
        {
            if (read_json_file(&client_socket, GAME_LIST_PATH, &response) < 0)
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
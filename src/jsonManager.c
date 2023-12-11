#include "../include/jsonManager.h"

int read_json_file(char *path_json_file, char **content)
{
    FILE *games_list_file = fopen(path_json_file, "r");
    if (!games_list_file)
        handle_error("fopen games_list_file \"r\"", -1);

    fseek(games_list_file, 0, SEEK_END);
    long tell = ftell(games_list_file);
    fseek(games_list_file, 0, SEEK_SET);

    *content = (char *)calloc(tell + 1, sizeof(char));
    if (!*content)
    {
        fclose(games_list_file);
        handle_error("Error allocating memory, read_json_file()", -1);
    }
    if (fread(*content, 1, tell, games_list_file) < 0)
    {
        free(*content);
        fclose(games_list_file);
        handle_error("read_json_file(): fread", -1);
    }

    (*content)[tell] = '\0';

    fclose(games_list_file);
    return 0;
}

int action_game_join(int *cFd, char *buffer)
{
    // Parse the gamejoin request
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

    // Get the name of the game the player wants to join
    cJSON *game_name = cJSON_GetObjectItemCaseSensitive(file_join_game, "name");
    if (!(cJSON_IsString(game_name)) || (game_name->valuestring == NULL))
        handle_error("game_name: bad usage", -1);

    // Modify the temporary game file json
    char *join_game_file = "json/";
    size_t len_path = strlen(join_game_file) + strlen(game_name->valuestring) + 1;
    char join_game_path[len_path];
    sprintf(join_game_path, "%s%s.json", join_game_file, game_name->valuestring);

    char *tmp_file = (char *)calloc(MAX_SIZE_MESSAGE, sizeof(char));
    if (read_json_file(join_game_path, &tmp_file) < 0)
        handle_error_noexit("tmp_file: read_json_file");

    cJSON *tmp_game = cJSON_Parse(tmp_file);
    if (tmp_game == NULL)
    {
        handle_json_error("tmp_game");
        cJSON_Delete(tmp_game);
        return -1;
    }

    cJSON *nbPlayers = cJSON_GetObjectItemCaseSensitive(tmp_game, "nbPlayers");
    cJSON_ReplaceItemInObjectCaseSensitive(tmp_game, "nbPlayers", cJSON_CreateNumber(nbPlayers->valueint + 1));

    cJSON *array_pl = cJSON_GetObjectItemCaseSensitive(tmp_game, "players");
    cJSON *join_player_data = cJSON_CreateObject();
    cJSON_AddNumberToObject(join_player_data, "id", *cFd);
    cJSON *join_player_pos = cJSON_GetObjectItemCaseSensitive(tmp_game, "startPos");
    cJSON_AddStringToObject(join_player_data, "pos", join_player_pos->valuestring);
    cJSON_ReplaceItemInObjectCaseSensitive(tmp_game, "startPos", cJSON_CreateString("3,2"));
    cJSON_AddItemToArray(array_pl, join_player_data);

    char *json_str = cJSON_Print(tmp_game);
    FILE *game = fopen(join_game_path, "w");
    if (game == NULL)
        handle_error("fopen game_join \"w\"", -1);

    printf("%s\n", json_str);
    fputs(json_str, game);
    fclose(game);

    // Update of the gameslist json file
    char *content = (char *)calloc(MAX_SIZE_MESSAGE, sizeof(char));
    if (read_json_file(GAME_LIST_PATH, &content) < 0)
        handle_error_noexit("content: read_json_file");

    cJSON *cj_games_list = cJSON_Parse(content);
    if (cj_games_list == NULL)
    {
        handle_json_error("cj_games_list: cJSON_Parse");
        cJSON_Delete(cj_games_list);
        return -1;
    }
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

    cJSON_Delete(cj_games_list);
    free(content);
    running_game(cFd, game_name->valuestring);
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
    if (read_json_file(game_path, &response) < 0)
        handle_error("running_game(): read_json_file()", -1);

    if (response != NULL)
        send(*cFd, response, strlen(response), 0);

    return 0;
}

int action_game_create(int *cFd, char *buffer)
{
    char *content = (char *)calloc(MAX_SIZE_MESSAGE, sizeof(char));
    if (read_json_file(GAME_LIST_PATH, &content) < 0)
        handle_error_noexit("content: read_json_file");

    // Parse the actual gamesList json file
    cJSON *root = cJSON_Parse(content);
    if (root == NULL)
    {
        handle_json_error("root");
        cJSON_Delete(root);
        free(content);
        return -1;
    }

    // Gets the json part of client's request
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
    FILE *game_list_json = fopen(GAME_LIST_PATH, "w");

    if (game_list_json == NULL)
        handle_error("fopen game_list_json \"w\"", -1);

    printf("%s\n", json_str);
    fputs(json_str, game_list_json);
    fclose(game_list_json);

    cJSON_Delete(root);
    running_game(cFd, new_game_name->valuestring);
    cJSON_Delete(new_game);
    free(content);

    return 0;
}

/*les positions seront traitées et transmises sous la forme d’une couple (x,y)
avec x le numéro de colonne et y le numéro de ligne.
la case d’origine est considéré être la case située dans le coin inférieur gauche.
*/
int action_player_move(int *cFd, char *buffer)
{
    // Gets the json part of client's request
    char buff[strlen(buffer) - 17];
    for (int i = 0; i < strlen(buffer) - 17; i++)
        buff[i] = buffer[i + 17];

    buff[strlen(buff) - 1] = '\0';

    // Parse the client request of creating new game
    cJSON *move = cJSON_Parse(buff);
    if (move == NULL)
    {
        handle_json_error("move");
        cJSON_Delete(move);
        return -1;
    }

    cJSON *dir = cJSON_GetObjectItemCaseSensitive(move, "move");
    // Tester si le mouvement est possible
    // prend startpos?
    if (strcmp(dir->valuestring, "up") == 0)
    {
        // int is_valid_move(int x, int y);
    }
    else if (strcmp(dir->valuestring, "down") == 0)
    {
        // int is_valid_move(int x, int y);
    }
    else if (strcmp(dir->valuestring, "left") == 0)
    {
        // int is_valid_move(int x, int y);
    }
    else if (strcmp(dir->valuestring, "right") == 0)
    {
        // int is_valid_move(int x, int y);
    }

    const char *new_file = "json/";
    const char *name_file = "position_update";
    size_t len_path = strlen(new_file) + strlen(name_file) + 1;
    char new_path[len_path];
    sprintf(new_path, "%s%s.json", new_file, name_file);

    FILE *position = fopen(new_path, "w");
    if (position == NULL)
        handle_error("position fopen", -1);

    cJSON *new_pos = cJSON_CreateObject();
    cJSON_AddNumberToObject(new_pos, "player", *cFd);
    cJSON_AddStringToObject(new_pos, "dir", dir->valuestring);

    char *json_print = cJSON_Print(new_pos);

    position = fopen(new_path, "w");
    if (position == NULL)
        handle_error("position fopen", -1);

    fputs(json_print, position);
    fclose(position);
    return 0;
}
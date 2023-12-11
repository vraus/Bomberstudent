#include "../include/game_running.h"

void add_to_dictionnary(struct game_infos *dict, int map_id, int id, const char *pos)
{
    struct Player *newPlayer = (struct Player *)malloc(sizeof(struct Player));
    newPlayer->id = id;
    strncpy(newPlayer->pos, pos, sizeof(newPlayer->pos));
    newPlayer->next = dict->players;
    dict->players = newPlayer;
    dict->mapId = map_id;
}

void get_game_info(char *file_name, char *game_name, struct game_infos *game_infos)
{
    // Ouverture du fichier
    FILE *game_file = fopen(file_name, "r");
    if (!game_file)
        handle_error_noexit("fopen game_file \"r\"");

    fseek(game_file, 0, SEEK_END);
    long tell = ftell(game_file);
    fseek(game_file, 0, SEEK_SET);

    char *content = (char *)calloc(tell + 1, sizeof(char));
    if (!content)
    {
        fclose(game_file);
        handle_error_noexit("Error allocating memory");
    }
    if (fread(content, 1, tell, game_file) < 0)
    {
        free(content);
        fclose(game_file);
        handle_error_noexit("Error reading the game_file: fread()");
    }

    content[tell] = '\0';
    fclose(game_file);

    FILE *game_list = fopen("json/gameslist.json", "r");
    if (!game_list)
        handle_error_noexit("fopen games_list_file\"r\"");

    fseek(game_list, 0, SEEK_END);
    long tell_map = ftell(game_list);
    fseek(game_list, 0, SEEK_SET);

    char *map_info = (char *)calloc(tell_map + 1, sizeof(char));
    if (!map_info)
    {
        fclose(game_list);
        handle_error_noexit("Error allocating memory");
    }
    if (fread(map_info, 1, tell_map, game_list) < 0)
    {
        free(map_info);
        fclose(game_list);
        handle_error_noexit("Error reading the game_file: fread()");
    }
    cJSON *game_list_content = cJSON_Parse(map_info);
    cJSON *games = cJSON_GetObjectItemCaseSensitive(game_list_content, "games");
    int map_id;
    if (games != NULL)
    {
        cJSON *element;
        cJSON_ArrayForEach(element, games)
        {
            cJSON *game_name_check = cJSON_GetObjectItemCaseSensitive(element, "name");
            if (cJSON_IsString(game_name_check) && game_name_check->valuestring != NULL)
            {
                if (strcmp(game_name_check->valuestring, game_name) == 0)
                {
                    cJSON *json_map_id = cJSON_GetObjectItemCaseSensitive(element, "mapId");
                    map_id = cJSON_GetNumberValue(json_map_id);
                    break;
                }
            }
        }
    }

    // Parsing
    cJSON *game_file_content = cJSON_Parse(content);
    cJSON *players_info = cJSON_GetObjectItemCaseSensitive(game_file_content, "players");
    if (players_info != NULL)
    {
        cJSON *player;
        cJSON_ArrayForEach(player, players_info)
        {
            cJSON *player_id = cJSON_GetObjectItemCaseSensitive(player, "id");
            cJSON *player_start_pos = cJSON_GetObjectItemCaseSensitive(player, "pos");

            if (player_id != NULL && player_start_pos != NULL)
            {
                // Convertir les valeurs JSON en entier et chaîne de caractères respectivement
                int id = cJSON_GetNumberValue(player_id);
                const char *pos = cJSON_GetStringValue(player_start_pos);

                // Ajouter le joueur à la liste
                add_to_dictionnary(game_infos, map_id, id, pos);
            }
        }
    }
}

void print_game_info(struct game_infos *game_infos)
{
    printf("Map ID: %d\n", game_infos->mapId);

    struct Player *current_player = game_infos->players;
    while (current_player != NULL)
    {
        printf("Player ID: %d, Position: %s\n", current_player->id, current_player->pos);
        current_player = current_player->next;
    }
}

void send_message_to_all(struct game_infos *game_infos, int sender_socket, const char *message)
{
    struct Player *current_player = game_infos->players;

    while (current_player != NULL)
    {

        cJSON *json_message = cJSON_CreateObject();
        cJSON_AddNumberToObject(json_message, "id", sender_socket);
        cJSON_AddStringToObject(json_message, "message", message);

        char *json_string = cJSON_Print(json_message);

        ssize_t sent_bytes = send(current_player->id, json_string, strlen(json_string), 0);

        if (sent_bytes < 0)
            handle_error_noexit("Error sending message to player");

        free(json_string);
        cJSON_Delete(json_message);

        current_player = current_player->next;
    }
}

int run_game(int *cFd, char *content)
{
    int client_socket = *cFd;
    ssize_t rd;
    char *buffer = (char *)calloc(MAX_SIZE_MESSAGE, sizeof(char));

    struct game_infos game_infos_instance = {0, NULL};

    // Extraction of game name in content
    char buff[strlen(content) - 15];
    for (int i = 0; i < strlen(content) - 15; ++i)
        buff[i] = content[i + 15];
    buff[strlen(buff) - 1] = '\0';

    // Parse the request message get the name
    cJSON *file_game_name = cJSON_Parse(buff);
    if (file_game_name == NULL)
    {
        handle_json_error("file_game_name");
        cJSON_Delete(file_game_name);
        return -1;
    }

    // Get the name of the game the player started
    cJSON *game_name = cJSON_GetObjectItemCaseSensitive(file_game_name, "name");
    if (!(cJSON_IsString(game_name)) || (game_name->valuestring == NULL))
        handle_error("game_name: bad usage", -1);

    // Retreive the game file
    char *game_file = "json/";
    size_t len_path = strlen(game_file) + strlen(game_name->valuestring) + 1;
    char start_game_path[len_path + 5];
    sprintf(start_game_path, "%s%s.json", game_file, game_name->valuestring);

    get_game_info(start_game_path, game_name->valuestring, &game_infos_instance);

    print_game_info(&game_infos_instance);

    // lire les messages du client et envoyer ses requêtes à tous les autres clients durant toute la partie
    while ((rd = read(client_socket, buffer, MAX_SIZE_MESSAGE)) > 0)
    {
        buffer[rd] = '\0';
        send_message_to_all(&game_infos_instance, client_socket, buffer);
    }

    return 0;
}
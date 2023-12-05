#ifndef JSONMANAGER_H
#define JSONMANAGER_H

#include "service.h"
#include "cJSON.h"

#define MAX_SIZE_MESSAGE 1024
#define GAME_LIST_PATH "json/gameslist.json"
#define MAPS_LIST_PATH "json/mapslist.json"

#define handle_json_error(msg)                       \
    do                                               \
    {                                                \
        const char *error_ptr = cJSON_GetErrorPtr(); \
        if (error_ptr != NULL)                       \
            handle_error_noexit("msg");              \
    } while (0)

/** @brief Reads a json file and sends it's content to the client file descriptor */
int read_json_file(char *path_json_file, char **content);

/** @brief Function to update the game file when a client connects to the game he chose. */
int action_game_join(int *cFd, char *buffer);

/** @brief Function to create the game file when client uses POST game/create. */
int create_running_game_data(char *game_name, int *cFd);

/** @brief This function is used to manage a running game with a client and the json file of the game.
 *  @note This function is yet to be done.
 */
int running_game(int *cFd, char *game_name);

/** @brief Function called when client send `POST game/create` request.
 *  modify the gamelist.json
 *  @return -1 when in error case (using the `handle_error` MACRO). 0 when no errors
 */
int action_game_create(int *cFd, char *buffer);

/** @brief Function called when client send `POST player/move` request.
 *  modify the gamelist.json
 *  @return -1 when in error case (using the `handle_error` MACRO). 0 when no errors
 */
int action_player_move(int *cFd, char *buffer);

#endif
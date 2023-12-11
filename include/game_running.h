#ifndef GAME_RUNNING_H
#define GAME_RUNNING_H

#include "jsonManager.h"

struct Player
{
    int id;
    char pos[10];
    struct Player *next;
};

struct game_infos
{
    int mapId;
    struct Player *players;
};

void add_to_dictionnary(struct game_infos *dict, int id, const char *pos);

void print_game_info(struct game_infos *game_infos);

void get_game_info(char *file_name, struct game_infos *game_infos);

int run_game(int *cFd, char *content);

#endif
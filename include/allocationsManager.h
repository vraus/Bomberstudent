#ifndef ALLOCATIONSMANAGER_H
#define ALLOCATIONSMANAGER_H

#include "service.h"
#include <signal.h>
#include "cJSON.h"

#define MAX_PSERVERS 50
#define MAX_PSTRINGS 1024
#define MAX_PFILES 100
#define MAX_PJSON_T 100

/**
 * @brief: Structure containing pointers to all allocated memory
 * @param pservers: `Server *` A pointer to all servers
 * @param num_pservers: `int` Count the number of pservers
 * @param pstrings: `char *` A pointer to all chars
 * @param num_pstrings: `int` Count the number of pstrings
 * @param pfiles: `FILE *` A pointer to all opened files
 * @param num_pfiles: `int` Count the number of pfiles
 * @param pjson_t: `json_t *` A pointer to all json types
 * @param num_psjon_t: `int` Count the number of pjson_t
 */
typedef struct
{
    Server **pservers;
    int num_pservers;
    char **pstrings;
    int num_pstrings;
    FILE **pfiles;
    int num_pfiles;
    // FIXME: Modify the structure to correspond to the new library
    // json_t **pjson_t;
    int num_pjson_t;
} AllocationManager;

/**
 * @brief: `int` Use this function to add a server to the AllocationManager
 * @param server: `Server*` A pointer to the server you want to add
 */
int add_server(Server *server);

/**
 * @brief: `int` Use this function whenever you want to close a Server
 * @param: `Server *` The pointer to the server you want to close
 */
int close_server(Server *server);

/**
 * @brief: `int` Use this function to add a string to the AllocationManager
 * @param string: `char *` A pointer to the string you want to add
 */
int add_string(char *string);

/**
 * @brief: `int` Use this function whenever you want to free a string
 * @param: `char *` The pointer to the string you want to free
 */
int free_string(char *string);

/**
 * @brief: `int` Use this function to add a FILE to the AllocationManager
 * @param string: `FILE *` A pointer to the FILE you want to add
 */
int add_file(FILE *file);

/**
 * @brief: `int` Use this function whenever you want to close a FILE
 * @param: `FILE *` The pointer to the FILE you want to close
 */
int fclose_file(FILE *string);

// FIXME: Modify those methods to correspond to the new library

/**
 * @brief: `int` Use this function to add a json_t to the AllocationManager
 * @param string: `json_t *` A pointer to the json_t you want to add
 */
// int add_json_t(json_t *pjson);

/**
 * @brief: `int` Use this function whenever you want to free a json_t
 * @param: `json_t *` The pointer to the json_t you want to free
 */
// int free_json_t(json_t *pjson_t);

/**
 * @brief: `void` Use this function to free all allocations in the AllocationManager
 */
void free_all_allocations(void);

#endif
#ifndef SERVICE_H
#define SERVICE_H

#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/select.h>

/**
 * @brief `int` Macro printing printing an error message and returning your error status
 * @param msg `char *` perror your message passed
 * @param status `int` return the value for error handling
 */
#define handle_error(msg, status) \
    do                            \
    {                             \
        perror(msg);              \
        return status;            \
    } while (0)

/**
 * @brief Structure for each Server
 * @param serverSocket `int` Server File Descriptor used for the socket
 * @param serverAddr `sockaddr_in` An instance of the sockaddr_in struct
 * @param protocol `char *` Set to `t` if it's a TCP server or `u` if it's a UDP server
 */
typedef struct
{
    int serverSocket;
    struct sockaddr_in serverAddr;
    char *protocol;
} Server;

/**
 * @brief Function used for the socket initialisation
 * @param server: `Server *` Pointer to the server struct
 * @param port: `int` port on which the server will be listening to
 */
int init_server(Server *server, int port);

#endif
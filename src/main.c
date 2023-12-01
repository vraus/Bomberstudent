#include "service.h"
#include "allocationsManager.h"
#include "jsonManager.h"

#include <pthread.h>

#define MAX_CLIENTS 200 // Max Number of pthread for clients

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
            if (read_json_file(GAME_LIST_PATH, &response) < 0)
                handle_error_noexit("GET game/create");
        }
        else if (strncmp(buffer, "POST game/join", 14) == 0)
        {
            action_game_join(&client_socket, buffer);
            if (read_json_file(GAME_LIST_PATH, &response) < 0)
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
        // XXX: problème avec le accept (regarder valgrind)
        if ((client_sockets[index] = accept(server_manager.server_socket, (struct sockaddr *)&c_addr, &c_addr_len)) < 0)
            handle_error("accept", -1);

        printf("New user joined: %d\n", ++num_clients);

        if (pthread_create(&threads[index], NULL, answer_server, (void *)&client_sockets[index]) != 0)
            handle_error("pthread_create", -1);

        index++;
    }

    return 0;
}
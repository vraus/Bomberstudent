#include "allocationsManager.h"

AllocationManager alloc_manager = {0};

int add_server(Server *server)
{
    if (alloc_manager.num_pservers < MAX_PSERVERS)
    {
        alloc_manager.num_pservers++;
        Server **new_pserver = realloc(alloc_manager.pservers, alloc_manager.num_pservers * sizeof(Server *));
        if (new_pserver != NULL)
            alloc_manager.pservers = new_pserver;
        else
            handle_error("new_pserver realloc", -1);
    }
    else
    {
        fprintf(stderr, "Maximum servers created. Can't create more!\n");
        return -1;
    }
    return 0;
}

int close_server(Server *server)
{
    for (int i = 0; i < alloc_manager.num_pservers; i++)
    {
        if (alloc_manager.pservers[i] == server)
        {
            close(alloc_manager.pservers[i]->server_socket);
            close(server->server_socket);
            free(alloc_manager.pservers[i]);
            free(server);
            alloc_manager.num_pservers--;
            Server **update_servers = realloc(alloc_manager.pservers, alloc_manager.num_pservers * sizeof(Server *));
            if (update_servers != NULL)
                alloc_manager.pservers = update_servers;
            else
                handle_error("update_servers realloc", -1);

            break;
        }
    }
    return 0;
}

int add_string(char *string)
{
    if (alloc_manager.num_pstrings < MAX_PSTRINGS)
    {
        alloc_manager.num_pstrings++;
        char **new_pstring = realloc(alloc_manager.pstrings, alloc_manager.num_pstrings * sizeof(char *));
        if (new_pstring != NULL)
            alloc_manager.pstrings = new_pstring;
        else
            handle_error("new_pstrings realloc", -1);
    }
    else
    {
        fprintf(stderr, "Maximum strings created. Can't add more!\n");
        return -1;
    }
    return 0;
}

int free_string(char *string)
{
    for (int i = 0; i < alloc_manager.num_pstrings; i++)
    {
        if (alloc_manager.pstrings[i] == string)
        {
            free(alloc_manager.pstrings[i]);
            free(string);
            alloc_manager.num_pstrings--;
            char **update_strings = realloc(alloc_manager.pstrings, alloc_manager.num_pstrings * sizeof(char *));
            if (update_strings != NULL)
                alloc_manager.pstrings = update_strings;
            else
                handle_error("update_strings realloc", -1);
        }
    }
    return 0;
}

int add_file(FILE *file)
{
    if (alloc_manager.num_pfiles < MAX_PFILES)
    {
        alloc_manager.num_pfiles++;
        FILE **new_pfile = realloc(alloc_manager.pfiles, alloc_manager.num_pfiles * sizeof(FILE *));
        if (new_pfile != NULL)
            alloc_manager.pfiles = new_pfile;
        else
            handle_error("new_pfile realloc", -1);
    }
    else
    {
        fprintf(stderr, "Maximum pfile created. Can't add more!\n");
        return -1;
    }
    return 0;
}

int fclose_file(FILE *file)
{
    for (int i = 0; i < alloc_manager.num_pfiles; i++)
    {
        if (alloc_manager.pfiles[i] == file)
        {
            fclose(alloc_manager.pfiles[i]);
            fclose(file);
            alloc_manager.num_pfiles--;
            FILE **update_files = realloc(alloc_manager.pfiles, alloc_manager.num_pfiles * sizeof(FILE *));
            if (update_files != NULL)
                alloc_manager.pfiles = update_files;
            else
                handle_error("update_file realloc", -1);
        }
    }
    return 0;
}

// FIXME: Modify the methods to correspond to the new library
/*
int add_json_t(json_t *file)
{
    if (alloc_manager.num_pjson_t < MAX_PFILES)
    {
        alloc_manager.num_pjson_t++;
        json_t **new_pjson_t = realloc(alloc_manager.pjson_t, alloc_manager.num_pjson_t * sizeof(json_t *));
        if (new_pjson_t != NULL)
            alloc_manager.pjson_t = new_pjson_t;
        else
            handle_error("new_pjson_t realloc", -1);
    }
    else
    {
        fprintf(stderr, "Maximum pfile created. Can't add more!\n");
        return -1;
    }
    return 0;
}

int free_json_t(json_t *pjson_t)
{
    for (int i = 0; i < alloc_manager.num_pjson_t; i++)
    {
        if (alloc_manager.pjson_t[i] == pjson_t)
        {
            free(alloc_manager.pjson_t[i]);
            free(pjson_t);
            alloc_manager.num_pjson_t--;
            json_t **update_json_t = realloc(alloc_manager.pjson_t, alloc_manager.num_pjson_t * sizeof(json_t *));
            if (update_json_t != NULL)
                alloc_manager.pjson_t = update_json_t;
            else
                handle_error("update_json_t realloc", -1);
        }
    }
    return 0;
}*/

// FIXME: Modify the free to correspond to the new library
void free_all_allocations(void)
{
    for (int i = 0; i < alloc_manager.num_pservers; i++)
        free(alloc_manager.pservers[i]);
    for (int i = 0; i < alloc_manager.num_pstrings; i++)
        free(alloc_manager.pstrings[i]);
    for (int i = 0; i < alloc_manager.num_pfiles; i++)
        fclose(alloc_manager.pfiles[i]);
    // for (int i = 0; i < alloc_manager.num_pjson_t; i++)
    //    free(alloc_manager.pjson_t[i]);

    free(alloc_manager.pservers);
    free(alloc_manager.pstrings);
    free(alloc_manager.pfiles);
    // free(alloc_manager.pjson_t);
    printf("\nEverything was freed\n");
}

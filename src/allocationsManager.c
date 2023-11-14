#include "allocationsManager.h"

AllocationManager allocManager = {0};

int add_server(Server *server)
{
    if (allocManager.num_pservers < MAX_PSERVERS)
    {
        allocManager.num_pservers++;
        Server **new_pserver = realloc(allocManager.pservers, allocManager.num_pservers * sizeof(Server *));
        if (new_pserver != NULL)
            allocManager.pservers = new_pserver;
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
    for (int i = 0; i < allocManager.num_pservers; i++)
    {
        if (allocManager.pservers[i] == server)
        {
            close(allocManager.pservers[i]->serverSocket);
            close(server->serverSocket);
            free(allocManager.pservers[i]);
            free(server);
            allocManager.num_pservers--;
            Server **update_servers = realloc(allocManager.pservers, allocManager.num_pservers * sizeof(Server *));
            if (update_servers != NULL)
                allocManager.pservers = update_servers;
            else
                handle_error("update_servers realloc", -1);

            break;
        }
    }
    return 0;
}

int add_string(char *string)
{
    if (allocManager.num_pstrings < MAX_PSTRINGS)
    {
        allocManager.num_pstrings++;
        char **new_pstring = realloc(allocManager.pstrings, allocManager.num_pstrings * sizeof(char *));
        if (new_pstring != NULL)
            allocManager.pstrings = new_pstring;
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
    for (int i = 0; i < allocManager.num_pstrings; i++)
    {
        if (allocManager.pstrings[i] == string)
        {
            free(allocManager.pstrings[i]);
            free(string);
            allocManager.num_pstrings--;
            char **update_strings = realloc(allocManager.pstrings, allocManager.num_pstrings * sizeof(char *));
            if (update_strings != NULL)
                allocManager.pstrings = update_strings;
            else
                handle_error("update_strings realloc", -1);
        }
    }
    return 0;
}

int add_file(FILE *file)
{
    if (allocManager.num_pfiles < MAX_PFILES)
    {
        allocManager.num_pfiles++;
        FILE **new_pfile = realloc(allocManager.pfiles, allocManager.num_pfiles * sizeof(FILE *));
        if (new_pfile != NULL)
            allocManager.pfiles = new_pfile;
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
    for (int i = 0; i < allocManager.num_pfiles; i++)
    {
        if (allocManager.pfiles[i] == file)
        {
            fclose(allocManager.pfiles[i]);
            fclose(file);
            allocManager.num_pfiles--;
            FILE **update_files = realloc(allocManager.pfiles, allocManager.num_pfiles * sizeof(FILE *));
            if (update_files != NULL)
                allocManager.pfiles = update_files;
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
    if (allocManager.num_pjson_t < MAX_PFILES)
    {
        allocManager.num_pjson_t++;
        json_t **new_pjson_t = realloc(allocManager.pjson_t, allocManager.num_pjson_t * sizeof(json_t *));
        if (new_pjson_t != NULL)
            allocManager.pjson_t = new_pjson_t;
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
    for (int i = 0; i < allocManager.num_pjson_t; i++)
    {
        if (allocManager.pjson_t[i] == pjson_t)
        {
            free(allocManager.pjson_t[i]);
            free(pjson_t);
            allocManager.num_pjson_t--;
            json_t **update_json_t = realloc(allocManager.pjson_t, allocManager.num_pjson_t * sizeof(json_t *));
            if (update_json_t != NULL)
                allocManager.pjson_t = update_json_t;
            else
                handle_error("update_json_t realloc", -1);
        }
    }
    return 0;
}*/

// FIXME: Modify the free to correspond to the new library
void free_all_allocations(void)
{
    for (int i = 0; i < allocManager.num_pservers; i++)
        free(allocManager.pservers[i]);
    for (int i = 0; i < allocManager.num_pstrings; i++)
        free(allocManager.pstrings[i]);
    for (int i = 0; i < allocManager.num_pfiles; i++)
        fclose(allocManager.pfiles[i]);
    // for (int i = 0; i < allocManager.num_pjson_t; i++)
    //    free(allocManager.pjson_t[i]);

    free(allocManager.pservers);
    free(allocManager.pstrings);
    free(allocManager.pfiles);
    // free(allocManager.pjson_t);
    printf("\nEverything was freed\n");
}

#pragma once

#include <pthread.h>
#include <stddef.h>

#include "window.h"

// Why would you want to open 1024 windows?
#define MAX_WINDOWS 1024

typedef struct {
    int socket;
    char* socket_path;

    pthread_t listener_thread;

    pthread_mutex_t windows_mutex;
    Window* windows[MAX_WINDOWS];
    size_t windows_count;
} Server;

Server* server_create(void);
void server_destroy(Server* server);

// Returns false on error
bool server_run(Server* server);

void server_lock_windows(Server* server);
void server_unlock_windows(Server* server);

Window** server_get_windows(Server* server);
size_t server_get_window_count(Server* server);

Window* server_top_window(Server* server);

/*
 * WARNING: this function DOES NOT lock window access. You'll have to lock
 *          it yourself.
 *
 * Returns false on error.
 */
bool server_raise_window(Server* server, Window* window);

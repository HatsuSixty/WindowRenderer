#ifndef WR_SERVER_H_
#define WR_SERVER_H_

#include <pthread.h>
#include <stddef.h>

#include "window.h"

// Why would you want to open 1024 windows?
#define MAX_WINDOWS 1024

typedef struct {
    int socket;
    pthread_t listener_thread;

    bool windows_used;
    Window* windows[MAX_WINDOWS];
    size_t windows_count;
} Server;

Server* server_create(void);
bool server_run(Server* server);
void server_destroy(Server* server);

void server_lock_windows(Server* server);
void server_unlock_windows(Server* server);

#endif // WR_SERVER_H_

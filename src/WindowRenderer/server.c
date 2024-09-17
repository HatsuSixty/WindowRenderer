#include "server.h"

#include "WindowRenderer.h"

#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <sys/stat.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <pthread.h>

#include "window.h"

#define LISTEN_QUEUE 20

bool file_exists(char const* file_path)
{
    struct stat buf = { 0 };
    return stat(file_path, &buf) == 0;
}

Server* server_create(void)
{
    Server* server = malloc(sizeof(*server));
    memset(server, 0, sizeof(*server));

    return server;
}

void server_destroy(Server* server)
{
    close(server->socket);

    for (size_t i = 0; i < server->windows_count; ++i) {
        window_destroy(server->windows[i]);
    }

    free(server);
}

static WindowRendererResponse server_create_window(Server* server, 
                                                   char const* title, int width, int height)
{
    server_lock_windows(server);
    
    WindowRendererResponse response = {
        .kind = WRRESP_ERROR,
        .error_kind = WRERROR_OK,
    };

    Window* window = window_create(title, width, height);

    if (window != NULL) {
        server->windows[server->windows_count++] = window;
        response = (WindowRendererResponse) {
            .kind = WRRESP_WINID,
            .window_id = window->id,
        };
    } else {
        response.error_kind = WRERROR_CREATE_FAILED;
    }

    server_unlock_windows(server);

    return response;
}

static WindowRendererResponse server_close_window(Server* server, int window_id)
{
    server_lock_windows(server);

    WindowRendererResponse response = {
        .kind = WRRESP_ERROR,
        .error_kind = WRERROR_OK,
    };

    bool id_valid = false;

    for (size_t i = 0; i < server->windows_count; i++) {
        if (server->windows[i]->id == window_id) {
            id_valid = true;

            if (!window_destroy(server->windows[i])) {
                response.error_kind = WRERROR_CLOSE_FAILED;
                break;
            }

            if (!(i >= server->windows_count)) {
                memmove(&server->windows[i], &server->windows[i+1], sizeof(void*));
            }
            server->windows_count -= 1;

            break;
        }
    }

    if (!id_valid)
        response.error_kind = WRERROR_INVALID_WINID;

    server_unlock_windows(server);
    return response;
}

static bool receive_command(int client_fd, WindowRendererCommand* command)
{
    int num_bytes_received = recv(client_fd, command, sizeof(*command), 0);

    if (num_bytes_received == -1) {
        fprintf(stderr, "ERROR: could not receive bytes from socket: %s\n",
                strerror(errno));
        return false;
    }

    if (num_bytes_received == 0) {
        fprintf(stderr, "[INFO] Connection closed by client\n");
        return false;
    }

    return true;
}

static bool send_response(int client_fd, WindowRendererResponse response)
{
    if (send(client_fd, &response, sizeof(response), 0) == -1) {
        fprintf(stderr, "ERROR: could not send data to the client: %s\n",
                strerror(errno));
        return false;
    }

    return true;
}

typedef struct {
    int client_fd;
    Server* server;
} HandleClientInfo;

static void* server_handle_client(HandleClientInfo* info)
{
    Server* server = info->server;
    int cfd = info->client_fd;

    WindowRendererCommand command;

    while (true) {
        if (!receive_command(cfd, &command))
            goto exit;

        WindowRendererResponse response = {
            .kind = WRRESP_ERROR,
            .error_kind = WRERROR_OK,
        };

        printf("[INFO] Received command\n");

        switch (command.kind) {

        case WRCMD_CREATE_WINDOW:
            response = 
                server_create_window(server, 
                                     command.window_title, command.window_width,
                                     command.window_height);
            break;

        case WRCMD_CLOSE_WINDOW:
            response = server_close_window(server, command.window_id);
            break;

        default:
            printf("  => ERROR: unknown command `%d`\n", command.kind);
            response.error_kind = WRERROR_INVALID_COMMAND;

        }

        if (!send_response(cfd, response))
            continue;
    }

exit:
    free(info);
    printf("Exiting `handle_client` thread...\n");
    return NULL;
}

static void* server_listener(Server* server)
{
    if (listen(server->socket, LISTEN_QUEUE) == -1) {
        fprintf(stderr, "ERROR: could not listen to socket: %s\n",
                strerror(errno));
        goto exit;
    }

    while (true) {
        printf("[INFO] Waiting for connection...\n");

        struct sockaddr_un client_addr;
        socklen_t client_len = sizeof(client_addr);
        int client_fd = accept(server->socket, (struct sockaddr*)&client_addr,
                               &client_len);
        if (client_fd == -1) {
            fprintf(stderr, "ERROR: could not accept connection: %s\n",
                    strerror(errno));
            continue;
        }

        HandleClientInfo* info = malloc(sizeof(*info));
        info->server = server;
        info->client_fd = client_fd;

        pthread_t handle_client_thread;
        int status
            = pthread_create(&handle_client_thread, NULL,
                             (void* (*)(void*)) & server_handle_client, info);
        if (status != 0) {
            fprintf(stderr, "ERROR: could not create handle_client thread\n");
            continue;
        }
    }

exit:
    printf("Exiting `server_listener` thread...\n");
    return NULL;
}

bool server_run(Server* server)
{
    if (file_exists(SOCKET_PATH)) {
        if (unlink(SOCKET_PATH) != 0) {
            fprintf(stderr, "ERROR: could not delete `%s`: %s\n", 
                    SOCKET_PATH, strerror(errno));
            return false;
        }
    }

    server->socket = socket(AF_UNIX, SOCK_STREAM, 0);
    if (server->socket == -1) {
        fprintf(stderr, "ERROR: could not create socket: %s\n", strerror(errno));
        return false;
    }

    struct sockaddr_un server_addr;
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sun_family = AF_UNIX;
    strncpy(server_addr.sun_path, SOCKET_PATH,
            sizeof(server_addr.sun_path) - 1);

    int opt = SO_REUSEADDR;
    setsockopt(server->socket, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

    int bind_result = bind(server->socket, (struct sockaddr*)&server_addr,
                           sizeof(server_addr));
    if (bind_result == -1) {
        fprintf(stderr, "ERROR: could not bind socket: %s\n", strerror(errno));
        close(server->socket);
        return false;
    }

    int status = pthread_create(&server->listener_thread, NULL,
                                (void* (*)(void*)) & server_listener, server);
    if (status != 0) {
        fprintf(stderr, "ERROR: could not create listener thread\n");
        close(server->socket);
        return false;
    }

    return true;
}

void server_lock_windows(Server *server)
{
    while (server->windows_used) asm("nop");
    server->windows_used = true;
}

void server_unlock_windows(Server *server)
{
    server->windows_used = false;
}

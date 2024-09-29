#include "server.h"

#include "WindowRenderer/windowrenderer.h"

#include <errno.h>
#include <pthread.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/un.h>
#include <unistd.h>

#include "log.h"
#include "session.h"
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

    server->socket_path = session_generate_socket_name();

    pthread_mutex_init(&server->windows_mutex, NULL);

    return server;
}

void server_destroy(Server* server)
{
    close(server->socket);

    for (size_t i = 0; i < server->windows_count; ++i) {
        window_destroy(server->windows[i]);
    }

    pthread_mutex_destroy(&server->windows_mutex);

    free(server);
}

static int server_find_window(Server* server, uint32_t id)
{
    for (size_t i = 0; i < server->windows_count; ++i) {
        if (server->windows[i]->id == id) {
            return i;
        }
    }
    return -1;
}

/*
 * WARNING: this function takes an index in the server->windows array
 *          instead of a window id!
 */
static void server_remove_window(Server* server, int window_index)
{
    if ((size_t)window_index < server->windows_count - 1) {
        memmove(&server->windows[window_index],
                &server->windows[window_index + 1],
                sizeof(void*));
    }
    server->windows_count -= 1;
}

static WindowRendererResponse server_create_window(Server* server,
                                                   char const* title, int width, int height)
{
    server_lock_windows(server);

    Window* window = window_create(title, width, height);
    server->windows[server->windows_count++] = window;

    WindowRendererResponse response = {
        .kind = WRRESP_WINID,
        .status = WRSTATUS_OK,
        .response = {
            .window_id = window->id,
        },
    };

    server_unlock_windows(server);

    return response;
}

static WindowRendererResponse server_close_window(Server* server, uint32_t window_id)
{
    server_lock_windows(server);

    WindowRendererResponse response = {
        .kind = WRRESP_EMPTY,
        .status = WRSTATUS_OK,
    };

    int index = server_find_window(server, window_id);
    if (index == -1) {
        response.status = WRSTATUS_INVALID_WINID;
        goto defer;
    }

    window_destroy(server->windows[index]);
    server_remove_window(server, index);

defer:
    server_unlock_windows(server);
    return response;
}

static WindowRendererResponse server_set_window_dma_buf(Server* server, uint32_t window_id,
                                                        WindowRendererDmaBuf dma_buf, int dma_buf_fd)
{
    server_lock_windows(server);

    WindowRendererResponse response = {
        .kind = WRRESP_EMPTY,
        .status = WRSTATUS_OK,
    };

    int index = server_find_window(server, window_id);
    if (index == -1) {
        response.status = WRSTATUS_INVALID_WINID;
        goto defer;
    }

    if (dma_buf_fd == -1) {
        response.status = WRSTATUS_INVALID_DMA_BUF_FD;
        goto defer;
    }

    if (dma_buf.width != server->windows[index]->width
        || dma_buf.height != server->windows[index]->height) {
        response.status = WRSTATUS_INVALID_DMA_BUF_SIZE;
        goto defer;
    }

    server->windows[index]->dma_buf = (WindowDmaBuf) {
        .present = true,
        .fd = dma_buf_fd,
        .width = dma_buf.width,
        .height = dma_buf.height,
        .format = dma_buf.format,
        .stride = dma_buf.stride,
    };

defer:
    server_unlock_windows(server);
    return response;
}

static bool receive_command(int client_fd, WindowRendererCommand* command, int* received_fd)
{
    struct msghdr message_header = { 0 };

    char control_message_buffer[CMSG_SPACE(sizeof(*received_fd))];
    memset(control_message_buffer, 0, sizeof(control_message_buffer));

    struct iovec io_vector = {
        .iov_base = command,
        .iov_len = sizeof(*command),
    };
    message_header.msg_iov = &io_vector;
    message_header.msg_iovlen = 1;

    message_header.msg_control = control_message_buffer;
    message_header.msg_controllen = sizeof(control_message_buffer);

    int num_bytes_received = recvmsg(client_fd, &message_header, 0);

    if (num_bytes_received == -1) {
        log_log(LOG_ERROR, "Could not receive bytes from socket: %s",
                strerror(errno));
        return false;
    }

    if (num_bytes_received == 0) {
        log_log(LOG_INFO, "Connection closed by client");
        return false;
    }

    if (message_header.msg_controllen > 0) {
        struct cmsghdr* control_message = CMSG_FIRSTHDR(&message_header);
        if (control_message
            && control_message->cmsg_level == SOL_SOCKET
            && control_message->cmsg_type == SCM_RIGHTS) {
            memcpy(received_fd, CMSG_DATA(control_message), sizeof(*received_fd));
        } else {
            *received_fd = -1;
        }
    } else {
        *received_fd = -1;
    }

    return true;
}

static bool send_response(int client_fd, WindowRendererResponse response)
{
    struct msghdr message_header = { 0 };

    struct iovec io_vector = {
        .iov_base = &response,
        .iov_len = sizeof(response),
    };

    message_header.msg_iov = &io_vector;
    message_header.msg_iovlen = 1;

    if (sendmsg(client_fd, &message_header, 0) == -1) {
        log_log(LOG_ERROR, "Could not send data to the client: %s",
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

    while (true) {
        WindowRendererCommand command;
        int command_fd;

        if (!receive_command(cfd, &command, &command_fd))
            goto exit;

        WindowRendererResponse response = {
            .kind = WRRESP_EMPTY,
            .status = WRSTATUS_OK,
        };

        log_log(LOG_INFO, "Received command");

        switch (command.kind) {

        case WRCMD_CREATE_WINDOW:
            log_log(LOG_INFO, "  > WRCMD_CREATE_WINDOW");
            response = server_create_window(server,
                                            command.command.create_window.title,
                                            command.command.create_window.width,
                                            command.command.create_window.height);
            break;

        case WRCMD_CLOSE_WINDOW:
            log_log(LOG_INFO, "  > WRCMD_CLOSE_WINDOW");
            response = server_close_window(server, command.command.close_window.window_id);
            break;

        case WRCMD_SET_WINDOW_DMA_BUF:
            log_log(LOG_INFO, "  > WRCMD_SET_WINDOW_DMA_BUF");
            response = server_set_window_dma_buf(server,
                                                 command.command.set_window_dma_buf.window_id,
                                                 command.command.set_window_dma_buf.dma_buf,
                                                 command_fd);
            break;

        default:
            log_log(LOG_ERROR, "  => ERROR: unknown command `%d`", command.kind);
            response.status = WRSTATUS_INVALID_COMMAND;
        }

        if (!send_response(cfd, response))
            continue;
    }

exit:
    free(info);
    log_log(LOG_INFO, "Exiting `handle_client` thread...");
    return NULL;
}

static void* server_listener(Server* server)
{
    if (listen(server->socket, LISTEN_QUEUE) == -1) {
        log_log(LOG_ERROR, "Could not listen to socket: %s",
                strerror(errno));
        goto exit;
    }

    while (true) {
        log_log(LOG_INFO, "Waiting for connection...");

        struct sockaddr_un client_addr;
        socklen_t client_len = sizeof(client_addr);
        int client_fd = accept(server->socket, (struct sockaddr*)&client_addr,
                               &client_len);
        if (client_fd == -1) {
            log_log(LOG_ERROR, "Could not accept connection: %s",
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
            log_log(LOG_ERROR, "Could not create handle_client thread");
            continue;
        }
    }

exit:
    log_log(LOG_INFO, "Exiting `server_listener` thread...");
    return NULL;
}

bool server_run(Server* server)
{
    if (file_exists(server->socket_path)) {
        if (unlink(server->socket_path) != 0) {
            log_log(LOG_ERROR, "Could not delete `%s`: %s",
                    server->socket_path, strerror(errno));
            return false;
        }
    }

    server->socket = socket(AF_UNIX, SOCK_STREAM, 0);
    if (server->socket == -1) {
        log_log(LOG_ERROR, "Could not create socket: %s",
                strerror(errno));
        return false;
    }

    struct sockaddr_un server_addr;
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sun_family = AF_UNIX;
    strncpy(server_addr.sun_path, server->socket_path,
            sizeof(server_addr.sun_path) - 1);

    int opt = SO_REUSEADDR;
    setsockopt(server->socket, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

    int bind_result = bind(server->socket, (struct sockaddr*)&server_addr,
                           sizeof(server_addr));
    if (bind_result == -1) {
        log_log(LOG_ERROR, "Could not bind socket: %s", strerror(errno));
        close(server->socket);
        return false;
    }

    int status = pthread_create(&server->listener_thread, NULL,
                                (void* (*)(void*)) & server_listener, server);
    if (status != 0) {
        log_log(LOG_ERROR, "Could not create listener thread");
        close(server->socket);
        return false;
    }

    return true;
}

void server_lock_windows(Server* server)
{
    pthread_mutex_lock(&server->windows_mutex);
}

void server_unlock_windows(Server* server)
{
    pthread_mutex_unlock(&server->windows_mutex);
}

Window** server_get_windows(Server* server)
{
    return server->windows;
}

size_t server_get_window_count(Server* server)
{
    return server->windows_count;
}

Window* server_top_window(Server* server)
{
    return server->windows[server->windows_count - 1];
}

/*
 * WARNING: this function DOES NOT lock window access. You'll have to lock
 * it yourself.
 *
 * Returns false on error.
 */
bool server_raise_window(Server* server, Window* window)
{
    int index = server_find_window(server, window->id);
    if (index == -1) {
        log_log(LOG_ERROR, "Failed to raise window of id `%d`", window->id);
        return false;
    }

    server_remove_window(server, index);
    server->windows[server->windows_count++] = window;

    return true;
}

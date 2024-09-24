#include "server.h"

#include "WindowRenderer.h"

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

            if (i < server->windows_count - 1) {
                memmove(&server->windows[i], &server->windows[i + 1], sizeof(void*));
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

static WindowRendererResponse server_set_window_dma_buf(Server* server, int window_id,
                                                        int dma_buf_fd, WindowRendererDmaBuf dma_buf)
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

            if (dma_buf_fd == -1) {
                response.error_kind = WRERROR_INVALID_DMA_BUF_FD;
                break;
            }

            if (dma_buf.width != server->windows[i]->width
                || dma_buf.height != server->windows[i]->height) {
                response.error_kind = WRERROR_INVALID_DMA_BUF_SIZE;
                break;
            }

            server->windows[i]->dma_buf = (WindowDmaBuf) {
                .present = true,
                .fd = dma_buf_fd,
                .width = dma_buf.width,
                .height = dma_buf.height,
                .format = dma_buf.format,
                .stride = dma_buf.stride,
            };

            break;
        }
    }

    if (!id_valid)
        response.error_kind = WRERROR_INVALID_WINID;

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
        fprintf(stderr, "ERROR: could not receive bytes from socket: %s\n",
                strerror(errno));
        return false;
    }

    if (num_bytes_received == 0) {
        fprintf(stderr, "[INFO] Connection closed by client\n");
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

    while (true) {
        WindowRendererCommand command;
        int command_fd;

        if (!receive_command(cfd, &command, &command_fd))
            goto exit;

        WindowRendererResponse response = {
            .kind = WRRESP_ERROR,
            .error_kind = WRERROR_OK,
        };

        printf("[INFO] Received command\n");

        switch (command.kind) {

        case WRCMD_CREATE_WINDOW:
            printf("  > WRCMD_CREATE_WINDOW\n");
            response = server_create_window(server,
                                            command.window_title, command.window_width,
                                            command.window_height);
            break;

        case WRCMD_CLOSE_WINDOW:
            printf("  > WRCMD_CLOSE_WINDOW\n");
            response = server_close_window(server, command.window_id);
            break;

        case WRCMD_SET_WINDOW_DMA_BUF:
            printf("  > WRCMD_SET_WINDOW_DMA_BUF\n");
            response = server_set_window_dma_buf(server, command.window_id, command_fd, command.dma_buf);
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
    if (file_exists(server->socket_path)) {
        if (unlink(server->socket_path) != 0) {
            fprintf(stderr, "ERROR: could not delete `%s`: %s\n",
                    server->socket_path, strerror(errno));
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
    strncpy(server_addr.sun_path, server->socket_path,
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

void server_lock_windows(Server* server)
{
    pthread_mutex_lock(&server->windows_mutex);
}

void server_unlock_windows(Server* server)
{
    pthread_mutex_unlock(&server->windows_mutex);
}

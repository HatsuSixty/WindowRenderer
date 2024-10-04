#include "window.h"

#include "log.h"
#include "event_list.h"
#include "session.h"

#include <errno.h>
#include <fcntl.h>
#include <pthread.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/un.h>
#include <unistd.h>

#include "WindowRenderer/windowrenderer.h"

#define LISTEN_QUEUE 20

static bool file_exists(char const* file_path)
{
    struct stat buf = { 0 };
    return stat(file_path, &buf) == 0;
}

static bool send_event(int client_fd, WindowRendererEvent event)
{
    struct msghdr message_header = { 0 };

    struct iovec io_vector = {
        .iov_base = &event,
        .iov_len = sizeof(event),
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

static void* event_listener(Window* window)
{
    int clientfd = -1;

    if (listen(window->event_socket, LISTEN_QUEUE) == -1) {
        log_log(LOG_ERROR, "Could not listen to socket: %s",
                strerror(errno));
        goto exit;
    }

    while (window->event_listener_thread_running) {
        struct sockaddr_un client_addr;
        socklen_t client_len = sizeof(client_addr);
        clientfd = accept(window->event_socket, (struct sockaddr*)&client_addr,
                          &client_len);
        if (clientfd == -1) {
            log_log(LOG_ERROR, "Could not accept connection: %s",
                    strerror(errno));
            continue;
        }

        log_log(LOG_INFO, "Client connected to event socket of window of ID %d",
                window->id);

        break;
    }

    while (window->event_listener_thread_running) {
        pthread_mutex_lock(&window->event_list_mutex);

        if (event_list_get_count(&window->event_list) != 0) {
            WindowRendererEvent event = event_list_pop(&window->event_list);
            send_event(clientfd, event);
        }

        pthread_mutex_unlock(&window->event_list_mutex);
    }

exit:
    if (clientfd != -1)
        close(clientfd);

    log_log(LOG_INFO, "Exiting `event_listener` thread for window of ID %d...", window->id);
    return NULL;
}

Window* window_create(char const* title, int width, int height)
{
    Window* window = malloc(sizeof(*window));
    memset(window, 0, sizeof(*window));

    window->id = session_generate_window_id();
    window->title = title;
    window->width = width;
    window->height = height;

    window->event_socket = -1;

    pthread_mutex_init(&window->event_list_mutex, NULL);

    bool event_socket_failed = false;

    char const* socket_path = session_generate_window_socket_name(window->id);

    if (file_exists(socket_path)) {
        if (unlink(socket_path) != 0) {
            log_log(LOG_WARNING, "Could not delete `%s`: %s",
                    socket_path, strerror(errno));
            event_socket_failed = true;
            goto defer;
        }
    }

    window->event_socket = socket(AF_UNIX, SOCK_STREAM, 0);
    if (window->event_socket == -1) {
        log_log(LOG_WARNING, "Could not create socket: %s",
                strerror(errno));
        event_socket_failed = true;
        goto defer;
    }

    struct sockaddr_un server_addr;
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sun_family = AF_UNIX;
    strncpy(server_addr.sun_path, socket_path,
            sizeof(server_addr.sun_path) - 1);

    if (bind(window->event_socket, (struct sockaddr*)&server_addr,
             sizeof(server_addr))
        == -1) {
        log_log(LOG_WARNING, "Could not bind socket: %s", strerror(errno));
        event_socket_failed = true;
        goto defer;
    }

    window->event_listener_thread_running = true;
    int status = pthread_create(&window->event_listener_thread, NULL,
                                (void* (*)(void*)) & event_listener, window);
    if (status != 0) {
        log_log(LOG_ERROR, "Could not create event listener thread for window of ID %d",
                window->id);
        event_socket_failed = true;
        window->event_listener_thread_running = false;
        goto defer;
    }

defer:
    if (event_socket_failed) {
        if (window->event_socket != -1) {
            close(window->event_socket);
            window->event_socket = -1;
        }

        log_log(LOG_WARNING, "Failed to create event socket. "
                             "Window of ID %d won't receive events",
                window->id);
    }
    return window;
}

void window_destroy(Window* window)
{
    if (window->event_listener_thread_running) {
        window->event_listener_thread_running = false;
        pthread_join(window->event_listener_thread, NULL);
    }

    pthread_mutex_destroy(&window->event_list_mutex);

    if (window->event_socket != -1) {
        close(window->event_socket);
    }

    free(window);
}

void window_send_event(Window* window, WindowRendererEvent event)
{
    if (!window->event_listener_thread_running) {
        log_log(LOG_WARNING, "Window of ID %d is not listening to events. "
                             "Not sending events...");
        return;
    }

    log_log(LOG_INFO, "Sending event of kind %d to window of ID %d",
            event.kind, window->id);

    pthread_mutex_lock(&window->event_list_mutex);
    event_list_push(&window->event_list, event);
    pthread_mutex_unlock(&window->event_list_mutex);
}

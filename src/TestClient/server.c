#include "server.h"

#include <errno.h>
#include <stdio.h>

#include <fcntl.h>
#include <sys/mman.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <unistd.h>

#include "WindowRenderer.h"

static bool send_command(int sockfd, WindowRendererCommand command)
{
    if (send(sockfd, &command, sizeof(command), 0) < 0) {
        fprintf(stderr, "ERROR: could not send command to the server: %s\n",
                strerror(errno));
        return false;
    }
    return true;
}

static bool recv_response(int sockfd, WindowRendererResponse* response)
{
    int num_of_bytes_recvd = recv(sockfd, response, sizeof(*response), 0);

    if (num_of_bytes_recvd <= 0) {
        fprintf(stderr, "ERROR: could not receive data from the server: %s\n",
                strerror(errno));
        return false;
    }

    return true;
}

static bool is_response_valid(char const* command, WindowRendererResponseKind expected,
                              WindowRendererResponse response)
{
    if (response.kind == WRRESP_ERROR && response.error_kind != WRERROR_OK) {
        fprintf(stderr, "ERROR: failed to %s: error code %d\n", command,
                response.error_kind);
        return false;
    }

    if (response.kind != expected) {
        fprintf(stderr, "ERROR: failed to %s: got unexpected response: response kind %d\n",
                command, response.kind);
        return false;
    }

    return true;
}

int server_create()
{
    int sockfd = socket(AF_UNIX, SOCK_STREAM, 0);
    if (sockfd == -1) {
        fprintf(stderr, "ERROR: could open socket: %s\n", strerror(errno));
        return -1;
    }

    struct sockaddr_un addr;
    memset(&addr, 0, sizeof(struct sockaddr_un));
    addr.sun_family = AF_UNIX;
    strncpy(addr.sun_path, SOCKET_PATH, sizeof(addr.sun_path) - 1);

    if (connect(sockfd, (struct sockaddr*)&addr, sizeof(struct sockaddr_un)) == -1) {
        fprintf(stderr, "ERROR: could not connect to socket: %s\n", strerror(errno));
        close(sockfd);
        return -1;
    }

    return sockfd;
}

void server_destroy(int serverfd)
{
    close(serverfd);
}

int server_create_window(int serverfd, char const* title, int width, int height)
{
    WindowRendererCommand command;
    command.kind = WRCMD_CREATE_WINDOW;
    command.window_width = width;
    command.window_height = height;
    memcpy(command.window_title, title, strlen(title));

    if (!send_command(serverfd, command))
        return -1;

    WindowRendererResponse response;
    if (!recv_response(serverfd, &response))
        return -1;

    if (!is_response_valid("create window", WRRESP_WINID, response))
        return -1;

    return response.window_id;
}

bool server_close_window(int serverfd, int id)
{
    WindowRendererCommand command;
    command.kind = WRCMD_CLOSE_WINDOW;
    command.window_id = id;

    if (!send_command(serverfd, command))
        return false;

    WindowRendererResponse response;
    if (!recv_response(serverfd, &response))
        return false;

    if (is_response_valid("close window", WRRESP_ERROR, response))
        return false;

    return true;
}

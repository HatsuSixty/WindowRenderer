#include <assert.h>
#include <errno.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>

#include <fcntl.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <unistd.h>

#include "WindowRenderer.h"

bool send_command(int sockfd, WindowRendererCommand command)
{
    if (send(sockfd, &command, sizeof(command), 0) < 0) {
        fprintf(stderr, "ERROR: could not send command to the server: %s\n", strerror(errno));
        return false;
    }
    return true;
}

bool recv_response(int sockfd, WindowRendererResponse* response)
{
    int num_of_bytes_recvd = recv(sockfd, response, sizeof(*response), 0);

    if (num_of_bytes_recvd <= 0) {
        fprintf(stderr, "ERROR: could not receive data from the server: %s\n", strerror(errno));
        return false;
    }

    return true;
}

int main(void)
{
    int sockfd = socket(AF_UNIX, SOCK_STREAM, 0);
    if (sockfd < 0) {
        fprintf(stderr, "ERROR: could open socket: %s\n", strerror(errno));
        return 1;
    }

    struct sockaddr_un addr;
    memset(&addr, 0, sizeof(struct sockaddr_un));
    addr.sun_family = AF_UNIX;
    strncpy(addr.sun_path, SOCKET_PATH, sizeof(addr.sun_path) - 1);

    if (connect(sockfd, (struct sockaddr*)&addr, sizeof(struct sockaddr_un)) < 0) {
        fprintf(stderr, "ERROR: could not connect to socket: %s\n", strerror(errno));
        return 1;
    }

    char const* window_title = "Hello, World";

    WindowRendererCommand create_window_cmd;
    create_window_cmd.kind = WRCMD_CREATE_WINDOW;
    create_window_cmd.window_width = 400;
    create_window_cmd.window_height = 400;
    memcpy(create_window_cmd.window_title, window_title, strlen(window_title));

    if (!send_command(sockfd, create_window_cmd))
        return 1;

    WindowRendererResponse create_window_resp;
    if (!recv_response(sockfd, &create_window_resp))
        return 1;
    assert(create_window_resp.kind == WRRESP_WINID);

    printf("Window ID: %d\n", create_window_resp.window_id);

    sleep(10);

    WindowRendererCommand close_window_cmd;
    close_window_cmd.kind = WRCMD_CLOSE_WINDOW;
    close_window_cmd.window_id = create_window_resp.window_id;

    if (!send_command(sockfd, close_window_cmd))
        return 1;

    WindowRendererResponse close_window_resp;
    if (!recv_response(sockfd, &close_window_resp))
        return 1;
    assert(close_window_resp.kind == WRRESP_ERROR
           && close_window_resp.error_kind == WRERROR_OK);

    close(sockfd);

    return 0;
}

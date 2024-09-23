#include "server.h"

#include "server_session.h"

#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <sys/mman.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <unistd.h>

#include "WindowRenderer.h"

static bool send_command(int sockfd, WindowRendererCommand command, int sent_fd)
{
    struct msghdr message_header = { 0 };

    struct iovec io_vector = {
        .iov_base = &command,
        .iov_len = sizeof(command),
    };
    message_header.msg_iov = &io_vector;
    message_header.msg_iovlen = 1;

    if (sent_fd != -1) {
        char control_message_buffer[CMSG_SPACE(sizeof(sent_fd))];
        memset(control_message_buffer, 0, sizeof(control_message_buffer));

        struct cmsghdr* control_message = (struct cmsghdr*)control_message_buffer;
        control_message->cmsg_level = SOL_SOCKET;
        control_message->cmsg_type = SCM_RIGHTS;
        control_message->cmsg_len = CMSG_LEN(sizeof(sent_fd));

        memcpy(CMSG_DATA(control_message), &sent_fd, sizeof(sent_fd));

        message_header.msg_control = control_message;
        message_header.msg_controllen = sizeof(control_message_buffer);
    }

    if (sendmsg(sockfd, &message_header, 0) == -1) {
        fprintf(stderr, "ERROR: could not send command to the server: %s\n",
                strerror(errno));
        return false;
    }

    return true;
}

static bool recv_response(int sockfd, WindowRendererResponse* response)
{
    struct msghdr message_header = { 0 };

    struct iovec io_vector = {
        .iov_base = response,
        .iov_len = sizeof(*response)
    };

    message_header.msg_iov = &io_vector;
    message_header.msg_iovlen = 1;

    int num_of_bytes_recvd = recvmsg(sockfd, &message_header, 0);

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
    if (!server_session_init()) {
        return -1;
    }

    int sockfd = socket(AF_UNIX, SOCK_STREAM, 0);
    if (sockfd == -1) {
        fprintf(stderr, "ERROR: could not open socket: %s\n", strerror(errno));
        return -1;
    }

    struct sockaddr_un addr;
    memset(&addr, 0, sizeof(struct sockaddr_un));
    addr.sun_family = AF_UNIX;
    strncpy(addr.sun_path,
            server_session_get_socket_name(), sizeof(addr.sun_path) - 1);

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

    if (!send_command(serverfd, command, -1))
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

    if (!send_command(serverfd, command, -1))
        return false;

    WindowRendererResponse response;
    if (!recv_response(serverfd, &response))
        return false;

    if (is_response_valid("close window", WRRESP_ERROR, response))
        return false;

    return true;
}

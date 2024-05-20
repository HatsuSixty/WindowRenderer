#include <assert.h>
#include <errno.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

#include <fcntl.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <sys/mman.h>
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

bool is_response_valid(char const* command, WindowRendererResponseKind expected,
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

int create_window(int serverfd, char const* title, int width, int height)
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

bool close_window(int serverfd, int id)
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

    int width = 400;
    int height = 400;

    int window_id = create_window(sockfd, "Hello, World", 400, 400);
    if (window_id == -1) return 1;

    char const* shm_name_prefix = "/WRWindow";
    size_t shm_name_length = strlen(shm_name_prefix) + 5; // 5 = 4 digits + NULL

    char* pixels_shm_name = malloc(shm_name_length);
    memset(pixels_shm_name, 0, shm_name_length);
    snprintf(pixels_shm_name, shm_name_length, "%s%d", shm_name_prefix, window_id);

    int pixels_shm_fd = shm_open(pixels_shm_name, O_RDWR, 0666);
    if (pixels_shm_fd == -1) {
        fprintf(stderr, "ERROR: could not open shared memory for window of ID %d: %s\n",
                window_id, strerror(errno));
        return 1;
    }

    size_t pixels_shm_size = width * height * 4;

    void* pixels = mmap(NULL, pixels_shm_size, PROT_READ | PROT_WRITE,
                        MAP_SHARED, pixels_shm_fd, 0);
    if (pixels == MAP_FAILED) {
        fprintf(stderr, "ERROR: could not mmap shared memory for window of ID %d: %s\n",
                window_id, strerror(errno));
        return 1;
    }

    for (size_t i = 0; i < pixels_shm_size / sizeof(uint32_t); ++i) {
        ((uint32_t*)pixels)[i] = 0xFF000000;
    }

    sleep(10);

    if (munmap(pixels, pixels_shm_size) == -1) {
        fprintf(stderr, "ERROR: could not munmap shared memory for window of ID %d: %s\n",
                window_id, strerror(errno));
        return 1;
    }

    if (close(pixels_shm_fd) == -1) {
        fprintf(stderr, "ERROR: could not close shared memory for window of ID %d: %s\n",
                window_id, strerror(errno));
        return 1;
    }

    free(pixels_shm_name);

    close_window(sockfd, window_id);

    close(sockfd);

    return 0;
}

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

#include "server.h"
#include "window.h"

int main(void)
{
    int serverfd = server_create();
    if (serverfd == -1) return 1;

    Window* window = window_create(serverfd, "Hello, World", 400, 400);
    if (window == NULL) return 1;

    for (size_t i = 0; i < window->pixels_shm_size / sizeof(uint32_t); ++i) {
        ((uint32_t*)window->pixels)[i] = 0xFF000000;
    }

    sleep(10);

    window_close(window);
    server_destroy(serverfd);

    return 0;
}

#include "LibWR/window.h"

#include "LibWR/server.h"

#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>
#include <unistd.h>

Window* window_create(int serverfd, char const* title, int width, int height)
{
    bool failed = false;

    Window* window = malloc(sizeof(*window));
    memset(window, 0, sizeof(*window));

    // Set private state
    window->serverfd = serverfd;

    // Set public state
    window->width = width;
    window->height = height;

    window->id = server_create_window(serverfd, title, width, height);
    if (window->id == -1) {
        failed = true;
        goto defer;
    }

defer:
    if (failed) {
        if (window->id != -1)
            server_close_window(serverfd, window->id);

        free(window);
        return NULL;
    }
    return window;
}

bool window_set_dma_buf(Window* window, WindowDmaBuf dma_buf)
{
    if (!server_set_window_dma_buf(window->serverfd, window->id, dma_buf))
        return false;
    return true;
}

bool window_close(Window* window)
{
    bool result = true;

    if (!server_close_window(window->serverfd, window->id)) {
        result = false;
    }

    free(window);

    return result;
}

#include "window.h"

#include "server.h"

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

    window->serverfd = serverfd;

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

bool window_close(Window* window)
{
    bool result = true;

    if (!server_close_window(window->serverfd, window->id)) {
        result = false;
    }

    free(window);

    return result;
}

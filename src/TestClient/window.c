#include "window.h"

#include "server.h"
#include "server_session.h"

#include <errno.h>
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

    // Create the window on the server
    window->id = server_create_window(serverfd, title, width, height);
    if (window->id == -1) {
        failed = true;
        goto defer;
    }

    window->serverfd = serverfd;

    // Get shared memory name
    window->pixels_shm_name = server_session_get_window_shm_name(window->id);

    // Open the shared memory
    window->pixels_shm_fd = shm_open(window->pixels_shm_name, O_RDWR, 0666);
    if (window->pixels_shm_fd == -1) {
        fprintf(stderr, "ERROR: could not open shared memory for window of ID %d: %s\n",
                window->id, strerror(errno));
        failed = true;
        goto defer;
    }

    // Set the size of the shared memory and map it
    window->pixels_shm_size = width * height * 4;

    window->pixels = mmap(NULL, window->pixels_shm_size, PROT_READ | PROT_WRITE,
                          MAP_SHARED, window->pixels_shm_fd, 0);
    if (window->pixels == MAP_FAILED) {
        fprintf(stderr, "ERROR: could not mmap shared memory for window of ID %d: %s\n",
                window->id, strerror(errno));
        failed = true;
        goto defer;
    }

defer:
    if (failed) {
        if (window->id != -1)
            server_close_window(serverfd, window->id);

        if (window->pixels != MAP_FAILED && window->pixels != NULL)
            munmap(window->pixels, window->pixels_shm_size);

        if (window->pixels_shm_fd != -1)
            close(window->pixels_shm_fd);

        free(window);
        return NULL;
    }
    return window;
}

bool window_close(Window* window)
{
    bool result = true;

    close(window->pixels_shm_fd);

    if (munmap(window->pixels, window->pixels_shm_size) == -1) {
        fprintf(stderr, "ERROR: could not munmap shared memory for window of ID %d: %s\n",
                window->id, strerror(errno));
        result = false;
    }

    if (!server_close_window(window->serverfd, window->id)) {
        result = false;
    }

    free(window);

    return result;
}

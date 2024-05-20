#include "window.h"

#include "server.h"

#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>
#include <unistd.h>

Window* window_create(int serverfd, char const* title, int width, int height)
{
    Window* window = malloc(sizeof(*window));
    memset(window, 0, sizeof(*window));

    window->id = server_create_window(serverfd, title, width, height);
    window->serverfd = serverfd;

    char const* shm_name_prefix = "/WRWindow";
    size_t shm_name_length = strlen(shm_name_prefix) + 5; // 5 = 4 digits + NULL

    window->pixels_shm_name = malloc(shm_name_length);
    memset(window->pixels_shm_name, 0, shm_name_length);
    snprintf(window->pixels_shm_name, shm_name_length, "%s%d", shm_name_prefix, window->id);

    window->pixels_shm_fd = shm_open(window->pixels_shm_name, O_RDWR, 0666);
    if (window->pixels_shm_fd == -1) {
        fprintf(stderr, "ERROR: could not open shared memory for window of ID %d: %s\n",
                window->id, strerror(errno));
        return NULL;
    }

    window->pixels_shm_size = width * height * 4;

    window->pixels = mmap(NULL, window->pixels_shm_size, PROT_READ | PROT_WRITE,
                          MAP_SHARED, window->pixels_shm_fd, 0);
    if (window->pixels == MAP_FAILED) {
        fprintf(stderr, "ERROR: could not mmap shared memory for window of ID %d: %s\n",
                window->id, strerror(errno));
        return NULL;
    }

    return window;
}

bool window_close(Window* window)
{
    if (munmap(window->pixels, window->pixels_shm_size) == -1) {
        fprintf(stderr, "ERROR: could not munmap shared memory for window of ID %d: %s\n",
                window->id, strerror(errno));
        free(window->pixels_shm_name);
        free(window);
        return false;
    }

    if (close(window->pixels_shm_fd) == -1) {
        fprintf(stderr, "ERROR: could not close shared memory for window of ID %d: %s\n",
                window->id, strerror(errno));
        free(window->pixels_shm_name);
        free(window);
        return false;
    }

    if (!server_close_window(window->serverfd, window->id)) {
        free(window->pixels_shm_name);
        free(window);
        return false;
    }

    free(window->pixels_shm_name);
    free(window);
    return true;
}
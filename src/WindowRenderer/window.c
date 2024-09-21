#include "window.h"

#include "session.h"

#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <unistd.h>

Window* window_create(char const* title, int width, int height)
{
    bool failed = false;

    Window* window = malloc(sizeof(*window));
    memset(window, 0, sizeof(*window));

    window->id = session_generate_window_id();
    window->title = title;
    window->width = width;
    window->height = height;
    window->pixels_shm_fd = -1;

    window->pixels_shm_size = width * height * 4;

    window->pixels_shm_name = session_generate_window_shm_name(window->id);
    window->pixels_shm_fd = shm_open(window->pixels_shm_name, O_CREAT | O_RDWR, 0666);
    if (window->pixels_shm_fd == -1) {
        fprintf(stderr, "ERROR: could not create shared memory for window of ID %d: %s\n",
                window->id, strerror(errno));
        failed = true;
        goto defer;
    }

    if (ftruncate(window->pixels_shm_fd, window->pixels_shm_size) == -1) {
        fprintf(stderr, "ERROR: could not truncate shared memory for window of ID %d: %s\n",
                window->id, strerror(errno));
        failed = true;
        goto defer;
    }

    window->pixels = (void*)mmap(NULL, window->pixels_shm_size, PROT_READ | PROT_WRITE,
                                 MAP_SHARED, window->pixels_shm_fd, 0);
    if (window->pixels == MAP_FAILED) {
        fprintf(stderr, "ERROR: could not mmap shared memory for window of ID %d: %s\n",
                window->id, strerror(errno));
        failed = true;
        goto defer;
    }

    memset(window->pixels, 0xFF, window->pixels_shm_size);

defer:
    if (failed) {
        if (window->pixels != MAP_FAILED && window->pixels != NULL)
            munmap(window->pixels, window->pixels_shm_size);

        if (window->pixels_shm_fd != -1)
            close(window->pixels_shm_fd);

        free(window);
        return NULL;
    }
    return window;
}

bool window_destroy(Window* window)
{
    bool result = true;

    if (munmap(window->pixels, window->pixels_shm_size) == -1) {
        fprintf(stderr, "ERROR: could not munmap shared memory for window of ID %d: %s\n",
                window->id, strerror(errno));
        result = false;
    }

    if (close(window->pixels_shm_fd) == -1) {
        fprintf(stderr,
                "ERROR: could not close shared memory file descriptor for "
                "window of ID %d: %s\n",
                window->id, strerror(errno));
        result = false;
    }

    if (shm_unlink(window->pixels_shm_name) == -1) {
        fprintf(stderr, "ERROR: could not unlink shared memory for window of ID %d: %s\n",
                window->id, strerror(errno));
        result = false;
    }

    free(window);

    return result;
}

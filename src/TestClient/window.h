#ifndef WINDOW_H_
#define WINDOW_H_

#include <stdbool.h>
#include <stddef.h>

typedef struct {
    int id;

    char* pixels_shm_name;
    int pixels_shm_fd;
    size_t pixels_shm_size;
    void* pixels;

    // private state
    int serverfd;
} Window;

// Returns NULL on error
Window* window_create(int serverfd, char const* title, int width, int height);
bool window_close(Window* window);

#endif // WINDOW_H_

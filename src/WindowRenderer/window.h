#ifndef WR_WINDOW_H_
#define WR_WINDOW_H_

#include <stdbool.h>
#include <stddef.h>

#define PIXEL_COMPONENTS 4

typedef struct {
    int id;
    int width;
    int height;
    char const* title;

    void* pixels;
    char* pixels_shm_name;
    size_t pixels_shm_size;
    int pixels_shm_fd;
} Window;

Window* window_create(char const* title, int width, int height);

// Returns NULL on error
bool window_destroy(Window* window);

#endif // WR_WINDOW_H_

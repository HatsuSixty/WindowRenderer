#ifndef WINDOW_H_
#define WINDOW_H_

#include <stdbool.h>
#include <stddef.h>

typedef struct {
    int fd;
    int width;
    int height;
    int format;
    int stride;
} WindowDmaBuf;

typedef struct {
    int id;
    int width;
    int height;

    // private state
    int serverfd;
} Window;

// Returns NULL on error
Window* window_create(int serverfd, char const* title, int width, int height);

// Returns false on error
bool window_set_dma_buf(Window* window, WindowDmaBuf dma_buf);

// Returns false on error
bool window_close(Window* window);

#endif // WINDOW_H_

#ifndef WR_WINDOW_H_
#define WR_WINDOW_H_

#include <stdbool.h>
#include <stddef.h>

typedef struct {
    bool present;

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
    char const* title;
    WindowDmaBuf dma_buf;
} Window;

Window* window_create(char const* title, int width, int height);
void window_destroy(Window* window);

#endif // WR_WINDOW_H_

#pragma once

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

typedef struct {
    bool present;

    int fd;
    int width;
    int height;
    int format;
    int stride;
} WindowDmaBuf;

typedef struct {
    uint32_t id;
    char const* title;
    WindowDmaBuf dma_buf;

    int x;
    int y;
    int width;
    int height;

    // This is not used by the server or anything.
    // This is just for the main application to keep
    // track of what windows are being dragged.
    bool is_dragging;
} Window;

Window* window_create(char const* title, int width, int height);
void window_destroy(Window* window);

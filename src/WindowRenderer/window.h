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
} Window;

Window* window_create(char const* title, int width, int height);
void window_destroy(Window* window);

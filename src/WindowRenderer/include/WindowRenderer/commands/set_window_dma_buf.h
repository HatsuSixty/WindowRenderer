#pragma once

#include <stdint.h>

typedef struct {
    int width;
    int height;
    int format;
    int stride;
} WindowRendererDmaBuf;

typedef struct {
    uint32_t window_id;
    WindowRendererDmaBuf dma_buf;
} WindowRendererSetWindowDmaBuf;

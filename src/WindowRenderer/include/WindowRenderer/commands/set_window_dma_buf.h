#ifndef WINDOWRENDERER_SET_WINDOW_DMA_BUF_H_
#define WINDOWRENDERER_SET_WINDOW_DMA_BUF_H_

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

#endif // WINDOWRENDERER_SET_WINDOW_DMA_BUF_H_

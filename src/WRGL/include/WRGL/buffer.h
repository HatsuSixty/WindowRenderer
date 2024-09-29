#pragma once

#include <stdint.h>

#include <gbm.h>
#include <EGL/egl.h>

#include <libwr.h>

typedef struct {
    int gpu_fd;
    struct gbm_device* gbm;
    struct gbm_bo* gbm_bo;
    EGLDisplay egl_display;
    WRDmaBuf dma_buf;
} WRGLBuffer;

WRGLBuffer* wrgl_buffer_create_from_window(int serverfd, char const* gpu_device,
                                           uint32_t window_id, int width, int height);
void wrgl_buffer_destroy(WRGLBuffer* wrgl_buffer);

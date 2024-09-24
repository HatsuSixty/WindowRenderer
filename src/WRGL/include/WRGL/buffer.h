#ifndef WRGL_BUFFER_H_
#define WRGL_BUFFER_H_

#include <stdint.h>

#include <gbm.h>
#include <EGL/egl.h>

#include <libwr.h>

typedef struct {
    int gpu_fd;
    struct gbm_device* gbm;
    struct gbm_bo* gbm_bo;
    EGLDisplay egl_display;
    WRDmaBuf dma_buf_info;
} WRGLBuffer;

WRGLBuffer* wrgl_buffer_create_from_window(int serverfd, uint32_t window_id,
                                           int width, int height);
void wrgl_buffer_destroy(WRGLBuffer* wrgl_buffer);

#endif // WRGL_BUFFER_H_

#include "WRGL/buffer.h"

#include <errno.h>
#include <fcntl.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <gbm.h>
#include <EGL/egl.h>

#include "glext.h"

#include <libwr.h>

WRGLBuffer* wrgl_buffer_create_from_window(int serverfd, uint32_t window_id,
                                           int width, int height)
{
    WRGLBuffer* wrgl_buffer = malloc(sizeof(*wrgl_buffer));
    memset(wrgl_buffer, 0, sizeof(*wrgl_buffer));

    bool failed = false;

    if (!glext_load_extensions()) {
        failed = true;
        goto defer;
    }

    // Open graphics card device
    wrgl_buffer->gpu_fd = open("/dev/dri/card1", O_RDWR | O_CLOEXEC);
    if (wrgl_buffer->gpu_fd == -1) {
        fprintf(stderr, "ERROR: could not open graphics card device: %s\n",
                strerror(errno));
        failed = true;
        goto defer;
    }

    // Create GBM device
    wrgl_buffer->gbm = gbm_create_device(wrgl_buffer->gpu_fd);
    if (!wrgl_buffer->gbm) {
        fprintf(stderr, "ERROR: failed to create GBM device\n");
        failed = true;
        goto defer;
    }

    // Check if the format we're gonna use for the creation of the
    // GBM Buffer Object is supported
    if (!gbm_device_is_format_supported(wrgl_buffer->gbm,
                                        GBM_FORMAT_XRGB8888,
                                        GBM_BO_USE_RENDERING | GBM_BO_USE_LINEAR)) {
        fprintf(stderr, "ERROR: `GBM_FORMAT_XRGB8888` "
                        "and `GBM_BO_USE_RENDERING` | `GBM_BO_USE_LINEAR` is not a supported format\n");
        failed = true;
        goto defer;
    }

    // Create GBM Buffer Object
    wrgl_buffer->gbm_bo = gbm_bo_create(wrgl_buffer->gbm,
                                        width, height,
                                        GBM_FORMAT_XRGB8888,
                                        GBM_BO_USE_RENDERING | GBM_BO_USE_LINEAR);
    if (!wrgl_buffer->gbm_bo) {
        fprintf(stderr, "ERROR: failed to create GBM buffer object\n");
        failed = true;
        goto defer;
    }

    // Create EGL display from the GBM device
    wrgl_buffer->egl_display = WRGL_eglGetPlatformDisplayEXT(EGL_PLATFORM_GBM_MESA,
                                                             wrgl_buffer->gbm, NULL);
    if (wrgl_buffer->egl_display == EGL_NO_DISPLAY) {
        fprintf(stderr, "ERROR: failed to get EGL display\n");
        failed = true;
        goto defer;
    }

    // Initialize EGL display
    if (eglInitialize(wrgl_buffer->egl_display, NULL, NULL) != EGL_TRUE) {
        fprintf(stderr, "ERROR: failed to initialize EGL display\n");
        failed = true;
        goto defer;
    }

    // Get DMA buffer information
    WRDmaBuf window_dma_buf = {
        .fd = gbm_bo_get_fd(wrgl_buffer->gbm_bo),
        .width = gbm_bo_get_width(wrgl_buffer->gbm_bo),
        .height = gbm_bo_get_height(wrgl_buffer->gbm_bo),
        .format = gbm_bo_get_format(wrgl_buffer->gbm_bo),
        .stride = gbm_bo_get_stride(wrgl_buffer->gbm_bo),
    };

    // Set DMA buffer for window
    if (!wr_set_window_dma_buf(serverfd, window_id, window_dma_buf)) {
        failed = true;
        goto defer;
    }

    wrgl_buffer->dma_buf_info = window_dma_buf;

defer:
    if (failed) {
        if (wrgl_buffer->egl_display != 0)
            eglTerminate(wrgl_buffer->egl_display);

        if (wrgl_buffer->gbm_bo != 0)
            gbm_bo_destroy(wrgl_buffer->gbm_bo);

        if (wrgl_buffer->gbm != 0)
            gbm_device_destroy(wrgl_buffer->gbm);

        if (wrgl_buffer->gpu_fd != 0)
            close(wrgl_buffer->gpu_fd);

        free(wrgl_buffer);

        return NULL;
    }

    return wrgl_buffer;
}

void wrgl_buffer_destroy(WRGLBuffer* wrgl_buffer)
{
    eglTerminate(wrgl_buffer->egl_display);

    gbm_bo_destroy(wrgl_buffer->gbm_bo);
    gbm_device_destroy(wrgl_buffer->gbm);

    close(wrgl_buffer->gpu_fd);

    free(wrgl_buffer);
}

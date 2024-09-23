#include "wrgl.h"

#include <assert.h>
#include <errno.h>
#include <fcntl.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <EGL/egl.h>
#include <EGL/eglext.h>
#include <GLES2/gl2.h>
#include <GLES2/gl2ext.h>
#include <gbm.h>

#include <LibWR/window.h>

// Error checking system
#define gl(name, ...)                        \
    do {                                     \
        gl_clear_errors();                   \
        gl##name(__VA_ARGS__);               \
        gl_check_errors(__FILE__, __LINE__); \
    } while (0);

#define gl_call(...)                         \
    do {                                     \
        gl_clear_errors();                   \
        __VA_ARGS__;                         \
        gl_check_errors(__FILE__, __LINE__); \
    } while (0);

void gl_clear_errors()
{
    while (glGetError() != GL_NO_ERROR)
        ;
}

void gl_check_errors(const char* file, int line)
{
    GLenum error;
    while ((error = glGetError()) != GL_NO_ERROR) {
        printf("%s:%d: ERROR: OpenGL error: error code 0x%X\n",
               file, line, error);
    }
}

// Macro for loading EGL/OpenGL functions inside wrgl_context_create_from_window
// This macro gets undefined after the function's definition
#define LOAD_PROC(type, name)                                                    \
    type name;                                                                   \
    do {                                                                         \
        name = (type)eglGetProcAddress(#name);                                   \
        if (!name) {                                                             \
            fprintf(stderr, "ERROR: support for the function `" #name "` is "    \
                            "required, but the function could not be loaded\n"); \
            failed = true;                                                       \
            goto defer;                                                          \
        }                                                                        \
    } while (0);

WRGLContext* wrgl_context_create_from_window(Window* window)
{
    // Initialize context
    WRGLContext* wrgl_context = malloc(sizeof(*wrgl_context));
    memset(wrgl_context, 0, sizeof(*wrgl_context));

    bool failed = false;

    // Load EGL extensions
    LOAD_PROC(PFNEGLGETPLATFORMDISPLAYEXTPROC, eglGetPlatformDisplayEXT);
    LOAD_PROC(PFNEGLCREATEIMAGEKHRPROC, eglCreateImageKHR);
    LOAD_PROC(PFNEGLDESTROYIMAGEKHRPROC, eglDestroyImageKHR);

    // Load OpenGL functions
    LOAD_PROC(PFNGLEGLIMAGETARGETTEXTURE2DOESPROC, glEGLImageTargetTexture2DOES);

    // Open graphics card device
    wrgl_context->gpu_fd = open("/dev/dri/card1", O_RDWR | O_CLOEXEC);
    if (wrgl_context->gpu_fd == -1) {
        fprintf(stderr, "ERROR: could not open graphics card device: %s\n",
                strerror(errno));
        failed = true;
        goto defer;
    }

    // Create GBM device
    wrgl_context->gbm = gbm_create_device(wrgl_context->gpu_fd);
    if (!wrgl_context->gbm) {
        fprintf(stderr, "ERROR: failed to create GBM device\n");
        failed = true;
        goto defer;
    }

    // Check if the format we're gonna use for the creation of the
    // GBM Buffer Object is supported
    if (!gbm_device_is_format_supported(wrgl_context->gbm,
                                        GBM_FORMAT_XRGB8888,
                                        GBM_BO_USE_RENDERING | GBM_BO_USE_LINEAR)) {
        fprintf(stderr, "ERROR: `GBM_FORMAT_XRGB8888` "
                        "and `GBM_BO_USE_RENDERING` | `GBM_BO_USE_LINEAR` is not a supported format\n");
        failed = true;
        goto defer;
    }

    // Create GBM Buffer Object
    wrgl_context->gbm_bo = gbm_bo_create(wrgl_context->gbm,
                                         window->width, window->height,
                                         GBM_FORMAT_XRGB8888,
                                         GBM_BO_USE_RENDERING | GBM_BO_USE_LINEAR);
    if (!wrgl_context->gbm_bo) {
        fprintf(stderr, "ERROR: failed to create GBM buffer object\n");
        failed = true;
        goto defer;
    }

    // Create EGL display from the GBM device
    wrgl_context->egl_display = eglGetPlatformDisplayEXT(EGL_PLATFORM_GBM_MESA,
                                                         wrgl_context->gbm, NULL);
    if (wrgl_context->egl_display == EGL_NO_DISPLAY) {
        fprintf(stderr, "ERROR: failed to get EGL display\n");
        failed = true;
        goto defer;
    }

    // Initialize EGL display
    if (eglInitialize(wrgl_context->egl_display, NULL, NULL) != EGL_TRUE) {
        fprintf(stderr, "ERROR: failed to initialize EGL display\n");
        failed = true;
        goto defer;
    }

    // Bind OpenGL API
    eglBindAPI(EGL_OPENGL_API);

    // Create EGL context
    wrgl_context->egl_context = eglCreateContext(wrgl_context->egl_display,
                                                 NULL, EGL_NO_CONTEXT, NULL);
    if (wrgl_context->egl_context == EGL_NO_CONTEXT) {
        fprintf(stderr, "ERROR: failed to create EGL context\n");
        failed = true;
        goto defer;
    }

    // Make the created context the current one
    eglMakeCurrent(wrgl_context->egl_display,
                   EGL_NO_SURFACE, EGL_NO_SURFACE,
                   wrgl_context->egl_context);

    // Get DMA buffer information
    WindowDmaBuf window_dma_buf = {
        .fd = gbm_bo_get_fd(wrgl_context->gbm_bo),
        .width = gbm_bo_get_width(wrgl_context->gbm_bo),
        .height = gbm_bo_get_height(wrgl_context->gbm_bo),
        .format = gbm_bo_get_format(wrgl_context->gbm_bo),
        .stride = gbm_bo_get_stride(wrgl_context->gbm_bo),
    };

    // Create EGL image out of the DMA buffer
    EGLint image_attrs[] = {
        EGL_WIDTH, window_dma_buf.width,
        EGL_HEIGHT, window_dma_buf.height,
        EGL_LINUX_DRM_FOURCC_EXT, window_dma_buf.format,
        EGL_DMA_BUF_PLANE0_FD_EXT, window_dma_buf.fd,
        EGL_DMA_BUF_PLANE0_OFFSET_EXT, 0,
        EGL_DMA_BUF_PLANE0_PITCH_EXT, window_dma_buf.stride,
        EGL_NONE
    };

    wrgl_context->egl_image = eglCreateImageKHR(wrgl_context->egl_display, EGL_NO_CONTEXT,
                                                EGL_LINUX_DMA_BUF_EXT, NULL, image_attrs);
    if (wrgl_context->egl_image == EGL_NO_IMAGE_KHR) {
        fprintf(stderr, "ERROR: could not create EGL image from GBM buffer\n");
        failed = true;
        goto defer;
    }

    // Create OpenGL texture out of the EGL image
    gl(GenTextures, 1, &wrgl_context->gl_texture);

    gl(BindTexture, GL_TEXTURE_2D, wrgl_context->gl_texture);

    gl(EGLImageTargetTexture2DOES, GL_TEXTURE_2D, wrgl_context->egl_image);

    gl(TexParameteri, GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    gl(TexParameteri, GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    gl(TexParameteri, GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    gl(TexParameteri, GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    gl(BindTexture, GL_TEXTURE_2D, 0);

    // Create OpenGL framebuffer out of the OpenGL texture
    gl(GenFramebuffers, 1, &wrgl_context->gl_framebuffer_object);
    gl(BindFramebuffer, GL_FRAMEBUFFER, wrgl_context->gl_framebuffer_object);

    gl(FramebufferTexture2D, GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D,
       wrgl_context->gl_texture, 0);

    GLuint framebuffer_status;
    gl_call(framebuffer_status = glCheckFramebufferStatus(GL_FRAMEBUFFER));
    if (framebuffer_status != GL_FRAMEBUFFER_COMPLETE) {
        fprintf(stderr, "ERROR: OpenGL framebuffer is not complete\n");
        failed = true;
        goto defer;
    }

    // Initialization complete!
    // The user may now use the available OpenGL context
    printf("[INFO] WRGL context initialized\n");

    // Set window DMA buffer
    window_set_dma_buf(window, window_dma_buf);

defer:
    if (failed) {
        if (wrgl_context->gl_framebuffer_object != 0)
            gl(DeleteFramebuffers, 1, &wrgl_context->gl_framebuffer_object);

        if (wrgl_context->gl_texture != 0)
            gl(DeleteTextures, 1, &wrgl_context->gl_texture);

        if (wrgl_context->egl_image != 0)
            eglDestroyImageKHR(wrgl_context->egl_context, wrgl_context->egl_image);

        if (wrgl_context->egl_context != 0)
            eglDestroyContext(wrgl_context->egl_display, wrgl_context->egl_context);

        if (wrgl_context->egl_display != 0)
            eglTerminate(wrgl_context->egl_display);

        if (wrgl_context->gbm_bo != 0)
            gbm_bo_destroy(wrgl_context->gbm_bo);

        if (wrgl_context->gbm != 0)
            gbm_device_destroy(wrgl_context->gbm);

        if (wrgl_context->gpu_fd != 0)
            close(wrgl_context->gpu_fd);

        free(wrgl_context);

        return NULL;
    }
    return wrgl_context;
}

// Undefine LOAD_PROC macro
#undef LOAD_PROC

void wrgl_context_destroy(WRGLContext* wrgl_context)
{
    PFNEGLDESTROYIMAGEKHRPROC eglDestroyImageKHR
        = (PFNEGLDESTROYIMAGEKHRPROC)eglGetProcAddress("eglDestroyImageKHR");
    assert(eglDestroyImageKHR != NULL);

    gl(DeleteFramebuffers, 1, &wrgl_context->gl_framebuffer_object);
    gl(DeleteTextures, 1, &wrgl_context->gl_texture);

    eglDestroyImageKHR(wrgl_context->egl_context, wrgl_context->egl_image);
    eglDestroyContext(wrgl_context->egl_display, wrgl_context->egl_context);
    eglTerminate(wrgl_context->egl_display);

    gbm_bo_destroy(wrgl_context->gbm_bo);
    gbm_device_destroy(wrgl_context->gbm);

    close(wrgl_context->gpu_fd);

    free(wrgl_context);
}

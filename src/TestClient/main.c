#include <assert.h>
#include <errno.h>
#include <fcntl.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/mman.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <unistd.h>

#include <EGL/egl.h>
#include <EGL/eglext.h>
#include <GL/gl.h>
#include <gbm.h>

#include "server.h"
#include "window.h"

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

#define LOAD_PROC(type, name)                                                   \
    type name;                                                                  \
    do {                                                                        \
        name = (type)eglGetProcAddress(#name);                                  \
        if (!name) {                                                            \
            fprintf(stderr, "ERROR: function `"#name"` support is required\n"); \
            return NULL;                                                        \
        }                                                                       \
    } while (0);

WindowDmaBuf* get_red_dma_buffer(int width, int height)
{
    int gpu_fd = open("/dev/dri/card1", O_RDWR | O_CLOEXEC);
    if (gpu_fd == -1) {
        fprintf(stderr, "ERROR: could not open graphics card device: %s\n",
                strerror(errno));
        return NULL;
    }

    struct gbm_device* gbm = gbm_create_device(gpu_fd);
    if (!gbm) {
        fprintf(stderr, "ERROR: failed to create GBM device\n");
        return NULL;
    }

    if (!gbm_device_is_format_supported(gbm,
                                        GBM_FORMAT_XRGB8888,
                                        GBM_BO_USE_RENDERING | GBM_BO_USE_LINEAR)) {
        fprintf(stderr, "ERROR: `GBM_FORMAT_XRGB8888` "
                        "and `GBM_BO_USE_RENDERING` | `GBM_BO_USE_LINEAR` is not a supported format\n");
        return NULL;
    }

    struct gbm_bo* gbm_bo = gbm_bo_create(gbm, width, height,
                                          GBM_FORMAT_XRGB8888,
                                          GBM_BO_USE_RENDERING | GBM_BO_USE_LINEAR);
    if (!gbm_bo) {
        fprintf(stderr, "ERROR: failed to create GBM buffer object\n");
        return NULL;
    }

    LOAD_PROC(PFNEGLGETPLATFORMDISPLAYEXTPROC, eglGetPlatformDisplayEXT);
    LOAD_PROC(PFNEGLCREATEIMAGEKHRPROC, eglCreateImageKHR);
    LOAD_PROC(PFNEGLDESTROYIMAGEKHRPROC, eglDestroyImageKHR);
    LOAD_PROC(PFNGLEGLIMAGETARGETTEXTURE2DOESPROC, glEGLImageTargetTexture2DOES);
    LOAD_PROC(PFNGLCHECKFRAMEBUFFERSTATUSPROC, glCheckFramebufferStatus);
    LOAD_PROC(PFNGLGENFRAMEBUFFERSPROC, glGenFramebuffers);
    LOAD_PROC(PFNGLBINDFRAMEBUFFERPROC, glBindFramebuffer);
    LOAD_PROC(PFNGLFRAMEBUFFERTEXTURE2DPROC, glFramebufferTexture2D);

    EGLDisplay egl_display = eglGetPlatformDisplayEXT(EGL_PLATFORM_GBM_MESA, gbm, NULL);
    if (egl_display == EGL_NO_DISPLAY) {
        fprintf(stderr, "ERROR: failed to get EGL display\n");
        return NULL;
    }

    EGLBoolean init_suceeded = eglInitialize(egl_display, NULL, NULL);
    if (init_suceeded != EGL_TRUE) {
        fprintf(stderr, "ERROR: failed to initialize EGL display\n");
        return NULL;
    }

    eglBindAPI(EGL_OPENGL_API);

    EGLContext egl_context = eglCreateContext(egl_display, NULL, EGL_NO_CONTEXT, NULL);
    if (egl_context == EGL_NO_CONTEXT) {
        fprintf(stderr, "ERROR: failed to create EGL context\n");
        return NULL;
    }

    eglMakeCurrent(egl_display, EGL_NO_SURFACE, EGL_NO_SURFACE, egl_context);

    WindowDmaBuf* window_dma_buf = malloc(sizeof(*window_dma_buf));
    memset(window_dma_buf, 0, sizeof(*window_dma_buf));

    window_dma_buf->width = gbm_bo_get_width(gbm_bo);
    window_dma_buf->height = gbm_bo_get_height(gbm_bo);
    window_dma_buf->format = gbm_bo_get_format(gbm_bo);
    window_dma_buf->fd = gbm_bo_get_fd(gbm_bo);
    window_dma_buf->stride = gbm_bo_get_stride(gbm_bo);

    EGLint image_attrs[] = {
        EGL_WIDTH, window_dma_buf->width,
        EGL_HEIGHT, window_dma_buf->height,
        EGL_LINUX_DRM_FOURCC_EXT, window_dma_buf->format,
        EGL_DMA_BUF_PLANE0_FD_EXT, window_dma_buf->fd,
        EGL_DMA_BUF_PLANE0_OFFSET_EXT, 0,
        EGL_DMA_BUF_PLANE0_PITCH_EXT, window_dma_buf->stride,
        EGL_NONE
    };

    EGLImageKHR egl_image = eglCreateImageKHR(egl_display, EGL_NO_CONTEXT,
                                              EGL_LINUX_DMA_BUF_EXT, NULL, image_attrs);
    if (egl_image == EGL_NO_IMAGE_KHR) {
        fprintf(stderr, "ERROR: could not create EGL image from GBM buffer\n");
        return NULL;
    }

    GLuint texture;
    gl(GenTextures, 1, &texture);
    gl(BindTexture, GL_TEXTURE_2D, texture);

    glEGLImageTargetTexture2DOES(GL_TEXTURE_2D, egl_image);

    gl(TexParameteri, GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    gl(TexParameteri, GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    gl(TexParameteri, GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    gl(TexParameteri, GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    GLuint fbo;
    gl(GenFramebuffers, 1, &fbo);
    gl(BindFramebuffer, GL_FRAMEBUFFER, fbo);

    gl(FramebufferTexture2D, GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, texture, 0);

    GLuint framebuffer_status;
    gl_call(framebuffer_status = glCheckFramebufferStatus(GL_FRAMEBUFFER));
    if (framebuffer_status != GL_FRAMEBUFFER_COMPLETE) {
        fprintf(stderr, "ERROR: framebuffer is not complete!\n");
    }

    gl(ClearColor, 1.0f, 0.0f, 0.0f, 1.0f);
    gl(Clear, GL_COLOR_BUFFER_BIT);

    gl(Flush);

    // eglDestroyImageKHR(egl_display, egl_image);
    // glDeleteTextures(1, &texture);
    // eglDestroyContext(egl_display, egl_context);
    // eglTerminate(egl_display);
    // gbm_bo_destroy(gbm_bo);
    // gbm_device_destroy(gbm);
    // close(gpu_fd);

    return window_dma_buf;
}

int main(void)
{
    int serverfd = server_create();
    if (serverfd == -1)
        return 1;

    int width = 400;
    int height = 400;

    Window* window = window_create(serverfd, "Hello, World", width, height);
    if (window == NULL)
        return 1;

    WindowDmaBuf* window_dma_buf = get_red_dma_buffer(width, height);
    if (window_dma_buf == NULL) {
        window_close(window);
        server_destroy(serverfd);
        return 1;
    }

    server_set_window_dma_buf(serverfd, window->id, *window_dma_buf);

    sleep(10);

    window_close(window);
    server_destroy(serverfd);

    return 0;
}

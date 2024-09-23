#ifndef WRGL_H_
#define WRGL_H_

#include <stdint.h>

#include <GLES2/gl2.h>
#include <GLES2/gl2ext.h>
#include <EGL/egl.h>
#include <EGL/eglext.h>
#include <gbm.h>

#include <LibWR/window.h>

typedef struct {
    int gpu_fd;

    struct gbm_device* gbm;
    struct gbm_bo* gbm_bo;

    EGLDisplay egl_display;
    EGLContext egl_context;
    EGLImageKHR egl_image;

    GLuint gl_framebuffer_object;
    GLuint gl_texture;
} WRGLContext;

WRGLContext* wrgl_context_create_from_window(Window* window);
void wrgl_context_destroy(WRGLContext* context);

#endif // WRGL_H_

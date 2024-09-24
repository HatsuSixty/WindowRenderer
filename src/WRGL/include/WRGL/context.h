#ifndef WRGL_CONTEXT_H_
#define WRGL_CONTEXT_H_

#include <EGL/egl.h>
#include <EGL/eglext.h>
#include <GLES2/gl2.h>
#include <GLES2/gl2ext.h>

#include "buffer.h"

typedef enum {
    WRGL_PROFILE_CORE,
    WRGL_PROFILE_COMPATIBILITY,
} WRGLContextProfile;

typedef struct {
    int major_version;
    int minor_version;
    WRGLContextProfile profile;
    bool debug;
    bool forward_compatible;
    bool robust_access;
} WRGLContextParameters;

typedef struct {
    EGLContext egl_context;
    EGLImageKHR egl_image;
    EGLDisplay egl_display;

    GLuint gl_framebuffer_object;
    GLuint gl_texture;
} WRGLContext;

WRGLContext* wrgl_context_create_for_buffer(WRGLBuffer* wrgl_buffer,
                                            WRGLContextParameters context_parameters);
void wrgl_context_destroy(WRGLContext* wrgl_context);

#endif // WRGL_CONTEXT_H_

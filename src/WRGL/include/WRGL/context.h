#pragma once

#include <EGL/egl.h>
#include <EGL/eglext.h>
#include <GLES2/gl2.h>
#include <GLES2/gl2ext.h>

#include "buffer.h"

typedef enum {
    WRGL_PROFILE_CORE,
    WRGL_PROFILE_COMPATIBILITY,
} WRGLContextProfile;

typedef enum {
    WRGL_API_OPENGL,
    WRGL_API_OPENGL_ES_1,
    WRGL_API_OPENGL_ES_2,
    WRGL_API_OPENGL_ES_3,
    WRGL_API_DONT_CARE,
} WRGLContextApiConformance;

typedef struct {
    // Context parameters
    int major_version;
    int minor_version;
    WRGLContextProfile profile;
    bool debug;
    bool forward_compatible;
    bool robust_access;

    // Frame buffer parameters
    WRGLContextApiConformance api_conformance;
    int red_bit_size;
    int green_bit_size;
    int blue_bit_size;
    int alpha_bit_size;
} WRGLContextParameters;

WRGLContextParameters wrgl_get_default_context_parameters();

typedef struct {
    EGLDisplay egl_display;
    EGLContext egl_context;
    EGLImageKHR egl_image;

    GLuint gl_texture;
    GLuint gl_framebuffer_object;
    GLuint gl_renderbuffer_object;
} WRGLContext;

WRGLContext* wrgl_context_create_for_buffer(WRGLBuffer* wrgl_buffer,
                                            WRGLContextParameters context_parameters);
void wrgl_context_destroy(WRGLContext* wrgl_context);

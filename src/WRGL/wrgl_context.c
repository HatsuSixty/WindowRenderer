#include "WRGL/context.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <EGL/egl.h>
#include <EGL/eglext.h>

#include "glext.h"
#include "log.h"

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

static void gl_clear_errors()
{
    while (glGetError() != GL_NO_ERROR)
        ;
}

static void gl_check_errors(const char* file, int line)
{
    GLenum error;
    while ((error = glGetError()) != GL_NO_ERROR) {
        log_log(LOG_ERROR, "%s:%d: OpenGL error: error code 0x%X",
                file, line, error);
    }
}

static EGLint wrgl_context_profile(WRGLContextProfile profile)
{
    switch (profile) {
    case WRGL_PROFILE_CORE:
        return EGL_CONTEXT_OPENGL_CORE_PROFILE_BIT;

    case WRGL_PROFILE_COMPATIBILITY:
        return EGL_CONTEXT_OPENGL_COMPATIBILITY_PROFILE_BIT;
    }

    return -1;
}

static EGLint wrgl_api_conformance(WRGLContextApiConformance api)
{
    switch (api) {
    case WRGL_API_OPENGL:
        return EGL_OPENGL_BIT;

    case WRGL_API_OPENGL_ES_1:
        return EGL_OPENGL_ES_BIT;

    case WRGL_API_OPENGL_ES_2:
        return EGL_OPENGL_ES2_BIT;

    case WRGL_API_OPENGL_ES_3:
        return EGL_OPENGL_ES3_BIT;

    case WRGL_API_DONT_CARE:
        return 0;
    }

    return -1;
}

WRGLContextParameters wrgl_get_default_context_parameters()
{
    return (WRGLContextParameters) {
        .major_version = 0,
        .minor_version = 0,
        .profile = WRGL_PROFILE_COMPATIBILITY,
        .debug = false,
        .forward_compatible = false,
        .robust_access = false,

        .api_conformance = WRGL_API_DONT_CARE,
        .red_bit_size = 0,
        .green_bit_size = 0,
        .blue_bit_size = 0,
        .alpha_bit_size = 0,
    };
}

WRGLContext* wrgl_context_create_for_buffer(WRGLBuffer* wrgl_buffer,
                                            WRGLContextParameters context_parameters)
{
    WRGLContext* wrgl_context = malloc(sizeof(*wrgl_context));
    memset(wrgl_context, 0, sizeof(*wrgl_context));

    wrgl_context->egl_display = wrgl_buffer->egl_display;

    bool failed = false;

    if (!glext_load_extensions()) {
        failed = true;
        goto defer;
    }

    EGLenum opengl_api;
    if (context_parameters.api_conformance == WRGL_API_OPENGL_ES_1
        || context_parameters.api_conformance == WRGL_API_OPENGL_ES_2
        || context_parameters.api_conformance == WRGL_API_OPENGL_ES_3)
        opengl_api = EGL_OPENGL_ES_API;
    else
        opengl_api = EGL_OPENGL_API;

    // Bind OpenGL API
    if (eglBindAPI(opengl_api) == EGL_FALSE) {
        log_log(LOG_ERROR, "Failed to set OpenGL API");
        failed = true;
        goto defer;
    }

    // Choose framebuffer configuration
    EGLint api_conformance = wrgl_api_conformance(context_parameters.api_conformance);
    if (api_conformance == -1) {
        log_log(LOG_ERROR, "Invalid API conformance");
        failed = true;
        goto defer;
    }

    EGLint frame_buffer_attributes[] = {
        EGL_CONFORMANT, api_conformance,
        EGL_RED_SIZE, context_parameters.red_bit_size,
        EGL_GREEN_SIZE, context_parameters.green_bit_size,
        EGL_BLUE_SIZE, context_parameters.blue_bit_size,
        EGL_ALPHA_SIZE, context_parameters.alpha_bit_size,
        EGL_NONE
    };

    EGLConfig egl_config;
    EGLint egl_config_size;

    if (eglChooseConfig(wrgl_context->egl_display,
                        frame_buffer_attributes, &egl_config, 1, &egl_config_size)
        != EGL_TRUE) {
        log_log(LOG_ERROR, "Failed to get EGL frame buffer configuration");
        failed = true;
        goto defer;
    }

    // Create EGL context
    EGLint context_major_version
        = context_parameters.major_version != 0 ? context_parameters.major_version : 1;
    EGLint context_minor_version
        = context_parameters.minor_version != 0 ? context_parameters.minor_version : 0;
    EGLBoolean context_forward_compatible
        = context_parameters.forward_compatible ? EGL_TRUE : EGL_FALSE;

    EGLint context_profile = wrgl_context_profile(context_parameters.profile);
    if (context_profile == -1) {
        log_log(LOG_ERROR, "Invalid context profile");
        failed = true;
        goto defer;
    }

    EGLint context_attributes[] = {
        EGL_CONTEXT_MAJOR_VERSION, context_major_version,
        EGL_CONTEXT_MINOR_VERSION, context_minor_version,
        EGL_CONTEXT_OPENGL_PROFILE_MASK, context_profile,
        EGL_CONTEXT_OPENGL_DEBUG, context_parameters.debug ? EGL_TRUE : EGL_FALSE,
        EGL_CONTEXT_OPENGL_FORWARD_COMPATIBLE, context_forward_compatible,
        EGL_CONTEXT_OPENGL_ROBUST_ACCESS, context_parameters.robust_access ? EGL_TRUE : EGL_FALSE,
        EGL_CONTEXT_OPENGL_RESET_NOTIFICATION_STRATEGY, EGL_NO_RESET_NOTIFICATION,
        EGL_NONE
    };

    wrgl_context->egl_context = eglCreateContext(wrgl_context->egl_display,
                                                 egl_config, EGL_NO_CONTEXT, context_attributes);
    if (wrgl_context->egl_context == EGL_NO_CONTEXT) {
        log_log(LOG_ERROR, "Failed to create EGL context");
        failed = true;
        goto defer;
    }

    // Make the created context the current one
    eglMakeCurrent(wrgl_context->egl_display,
                   EGL_NO_SURFACE, EGL_NO_SURFACE,
                   wrgl_context->egl_context);

    // Create EGL image out of the DMA buffer
    EGLint image_attrs[] = {
        EGL_WIDTH, wrgl_buffer->dma_buf.width,
        EGL_HEIGHT, wrgl_buffer->dma_buf.height,
        EGL_LINUX_DRM_FOURCC_EXT, wrgl_buffer->dma_buf.format,
        EGL_DMA_BUF_PLANE0_FD_EXT, wrgl_buffer->dma_buf.fd,
        EGL_DMA_BUF_PLANE0_OFFSET_EXT, 0,
        EGL_DMA_BUF_PLANE0_PITCH_EXT, wrgl_buffer->dma_buf.stride,
        EGL_NONE
    };

    wrgl_context->egl_image = eglCreateImageKHR(wrgl_context->egl_display, EGL_NO_CONTEXT,
                                                EGL_LINUX_DMA_BUF_EXT, NULL, image_attrs);
    if (wrgl_context->egl_image == EGL_NO_IMAGE_KHR) {
        log_log(LOG_ERROR, "Could not create EGL image from GBM buffer");
        failed = true;
        goto defer;
    }

    // Create OpenGL texture out of the EGL image
    gl(GenTextures, 1, &wrgl_context->gl_texture);

    gl(BindTexture, GL_TEXTURE_2D, wrgl_context->gl_texture);

    gl_clear_errors();
    glEGLImageTargetTexture2DOES(GL_TEXTURE_2D, wrgl_context->egl_image);
    gl_check_errors(__FILE__, __LINE__);

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

    GLuint framebuffer_status1;
    gl_call(framebuffer_status1 = glCheckFramebufferStatus(GL_FRAMEBUFFER));
    if (framebuffer_status1 != GL_FRAMEBUFFER_COMPLETE) {
        log_log(LOG_ERROR, "OpenGL framebuffer is not complete");
        failed = true;
        goto defer;
    }

    // Check support for packed depth + stencil renderbuffer
    char const* opengl_extensions = (char const*)glGetString(GL_EXTENSIONS);
    if (!(opengl_extensions
          && (strstr(opengl_extensions, "GL_EXT_packed_depth_stencil")
              || strstr(opengl_extensions, "GL_OES_packed_depth_stencil")))) {
        log_log(LOG_ERROR, "Support for the EGL extension `GL_EXT_packed_depth_stencil` "
                           "is required, but the extension is not in the supported extension list");
        failed = true;
        goto defer;
    }

    // Create OpenGL depth/stencil renderbuffer and attach it to the framebuffer
    gl(GenRenderbuffers, 1, &wrgl_context->gl_renderbuffer_object);
    gl(BindRenderbuffer, GL_RENDERBUFFER, wrgl_context->gl_renderbuffer_object);
    gl(RenderbufferStorage, GL_RENDERBUFFER, GL_DEPTH24_STENCIL8_OES, wrgl_buffer->dma_buf.width,
       wrgl_buffer->dma_buf.height);
    gl(FramebufferRenderbuffer, GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER,
       wrgl_context->gl_renderbuffer_object);

    gl(FramebufferRenderbuffer, GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER,
       wrgl_context->gl_renderbuffer_object);
    gl(FramebufferRenderbuffer, GL_FRAMEBUFFER, GL_STENCIL_ATTACHMENT, GL_RENDERBUFFER,
       wrgl_context->gl_renderbuffer_object);

    // Unbind renderbuffer
    gl(BindRenderbuffer, GL_RENDERBUFFER, 0);

    // Check framebuffer status again
    GLuint framebuffer_status2;
    gl_call(framebuffer_status2 = glCheckFramebufferStatus(GL_FRAMEBUFFER));
    if (framebuffer_status2 != GL_FRAMEBUFFER_COMPLETE) {
        log_log(LOG_ERROR, "OpenGL framebuffer is not complete");
        failed = true;
        goto defer;
    }

    // Initialization complete!
    // The user may now use the available OpenGL context
    log_log(LOG_INFO, "WRGL context initialized");

defer:
    if (failed) {
        if (wrgl_context->gl_renderbuffer_object != 0)
            gl(DeleteRenderbuffers, 1, &wrgl_context->gl_renderbuffer_object);

        if (wrgl_context->gl_framebuffer_object != 0)
            gl(DeleteFramebuffers, 1, &wrgl_context->gl_framebuffer_object);

        if (wrgl_context->gl_texture != 0)
            gl(DeleteTextures, 1, &wrgl_context->gl_texture);

        if (wrgl_context->egl_image != 0)
            eglDestroyImageKHR(wrgl_context->egl_context, wrgl_context->egl_image);

        if (wrgl_context->egl_context != 0)
            eglDestroyContext(wrgl_context->egl_display, wrgl_context->egl_context);

        free(wrgl_context);

        return NULL;
    }

    return wrgl_context;
}

void wrgl_context_destroy(WRGLContext* wrgl_context)
{
    gl(DeleteRenderbuffers, 1, &wrgl_context->gl_renderbuffer_object);
    gl(DeleteFramebuffers, 1, &wrgl_context->gl_framebuffer_object);
    gl(DeleteTextures, 1, &wrgl_context->gl_texture);

    eglDestroyImageKHR(wrgl_context->egl_context, wrgl_context->egl_image);
    eglDestroyContext(wrgl_context->egl_display, wrgl_context->egl_context);

    free(wrgl_context);
}

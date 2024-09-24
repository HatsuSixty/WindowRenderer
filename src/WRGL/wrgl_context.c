#include "WRGL/context.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "glext.h"

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

WRGLContext* wrgl_context_create_for_buffer(WRGLBuffer* wrgl_buffer)
{
    WRGLContext* wrgl_context = malloc(sizeof(*wrgl_context));
    memset(wrgl_context, 0, sizeof(*wrgl_context));

    wrgl_context->egl_display = wrgl_buffer->egl_display;

    bool failed = false;

    if (!glext_load_extensions()) {
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

    wrgl_context->egl_image = WRGL_eglCreateImageKHR(wrgl_context->egl_display, EGL_NO_CONTEXT,
                                                     EGL_LINUX_DMA_BUF_EXT, NULL, image_attrs);
    if (wrgl_context->egl_image == EGL_NO_IMAGE_KHR) {
        fprintf(stderr, "ERROR: could not create EGL image from GBM buffer\n");
        failed = true;
        goto defer;
    }

    // Create OpenGL texture out of the EGL image
    gl(GenTextures, 1, &wrgl_context->gl_texture);

    gl(BindTexture, GL_TEXTURE_2D, wrgl_context->gl_texture);

    gl_clear_errors();
    WRGL_glEGLImageTargetTexture2DOES(GL_TEXTURE_2D, wrgl_context->egl_image);
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

defer:
    if (failed) {
        if (wrgl_context->gl_framebuffer_object != 0)
            gl(DeleteFramebuffers, 1, &wrgl_context->gl_framebuffer_object);

        if (wrgl_context->gl_texture != 0)
            gl(DeleteTextures, 1, &wrgl_context->gl_texture);

        if (wrgl_context->egl_image != 0)
            WRGL_eglDestroyImageKHR(wrgl_context->egl_context, wrgl_context->egl_image);

        if (wrgl_context->egl_context != 0)
            eglDestroyContext(wrgl_context->egl_display, wrgl_context->egl_context);

        free(wrgl_context);

        return NULL;
    }
    return wrgl_context;
}

void wrgl_context_destroy(WRGLContext* wrgl_context)
{
    gl(DeleteFramebuffers, 1, &wrgl_context->gl_framebuffer_object);
    gl(DeleteTextures, 1, &wrgl_context->gl_texture);

    WRGL_eglDestroyImageKHR(wrgl_context->egl_context, wrgl_context->egl_image);
    eglDestroyContext(wrgl_context->egl_display, wrgl_context->egl_context);

    free(wrgl_context);
}

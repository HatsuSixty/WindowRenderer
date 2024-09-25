#include "application.h"

#include "log.h"
#include "renderer/glext.h"
#include "renderer/opengl/gl_errors.h"
#include "renderer/opengl/texture.h"
#include "renderer/renderer.h"
#include "session.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

bool execute_command(int argc, char const** argv, int delay)
{
    if (delay != 0) {
        for (int i = delay; i != 0; --i) {
            log_log(LOG_INFO, "Executing command in %d seconds", i);
            sleep(1);
        }
    }

    pid_t pid = fork();
    if (pid == -1) {
        log_log(LOG_ERROR, "Failed to fork child process");
        return false;
    }

    if (pid == 0) {
        char const* args[argc + 1];
        for (int i = 0; i < argc; ++i) {
            args[i] = argv[i];
        }
        args[argc] = NULL;

        execvp(args[0], (char** const)args);
        log_log(LOG_ERROR, "Failed to execute command `%s`", argv[0]);
        return false;
    }

    return true;
}

Application* application_create(int argc, char const** argv)
{
    char const* command = NULL;
    if (argc > 1)
        command = argv[1];

    Application* application = malloc(sizeof(*application));
    memset(application, 0, sizeof(*application));

    if (!glext_load_extensions()) {
        return NULL;
    }

    log_log(LOG_INFO, "Initializing WindowRenderer");

    session_init();

    log_log(LOG_INFO, "Session hash: %s", session_get_hash());

    application->server = server_create();
    if (!server_run(application->server))
        return NULL;

    if (!execute_command(1, (char const*[]) { command }, 5))
        return NULL;

    return application;
}

void application_destroy(Application* application)
{
    server_destroy(application->server);
    free(application);
}

bool application_init_graphics(Application* application, int width, int height)
{
    application->renderer = renderer_create(width, height);
    if (!application->renderer)
        return false;
    return true;
}

void application_destroy_graphics(Application* application)
{
    renderer_destroy(application->renderer);
}

void application_resize(Application* application, int width, int height)
{
    renderer_resize(application->renderer, width, height);
}

void application_render(Application* application, EGLDisplay* egl_display)
{
    (void)egl_display;

    gl(ClearColor, 0.8f, 0.8f, 0.8f, 1.0f);
    gl(Clear, GL_COLOR_BUFFER_BIT);

    renderer_begin_drawing(application->renderer);

    server_lock_windows(application->server);

    for (size_t i = 0; i < application->server->windows_count; ++i) {
        Window* window = application->server->windows[i];

        renderer_draw_rectangle(application->renderer,
                                (Vector2) { 0, 0 },
                                (Vector2) { window->width, window->height },
                                (Vector4) { 1.0f, 1.0f, 1.0f, 1.0f });

        if (window->dma_buf.present) {
            EGLint image_attrs[] = {
                EGL_WIDTH, window->dma_buf.width,
                EGL_HEIGHT, window->dma_buf.height,
                EGL_LINUX_DRM_FOURCC_EXT, window->dma_buf.format,
                EGL_DMA_BUF_PLANE0_FD_EXT, window->dma_buf.fd,
                EGL_DMA_BUF_PLANE0_OFFSET_EXT, 0,
                EGL_DMA_BUF_PLANE0_PITCH_EXT, window->dma_buf.stride,
                EGL_NONE
            };

            EGLImageKHR egl_image = eglCreateImageKHR(egl_display, EGL_NO_CONTEXT,
                                                      EGL_LINUX_DMA_BUF_EXT,
                                                      NULL,
                                                      image_attrs);
            if (egl_image == EGL_NO_IMAGE_KHR) {
                log_log(LOG_ERROR, "Could not create EGL image from DMA buffer");
                continue;
            }

            Texture* texture = texture_create_from_egl_imagekhr(egl_image,
                                                                window->dma_buf.width,
                                                                window->dma_buf.height);

            renderer_draw_texture_ex(application->renderer,
                                     texture,
                                     (Vector2) { 0, 0 },
                                     (Vector2) {
                                         window->dma_buf.width,
                                         window->dma_buf.height,
                                     },
                                     (Vector4) { 1.0f, 1.0f, 1.0f, 1.0f });

            texture_destroy(texture);

            eglDestroyImageKHR(egl_display, egl_image);
        }
    }

    server_unlock_windows(application->server);
}

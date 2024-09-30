#include "application.h"

#include "input.h"
#include "log.h"
#include "renderer/glext.h"
#include "renderer/opengl/gl_errors.h"
#include "renderer/opengl/texture.h"
#include "renderer/renderer.h"
#include "server.h"
#include "session.h"
#include "wm.h"

#include <errno.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

static bool execute_command(int argc, char const** argv, int delay)
{
    pid_t pid = fork();
    if (pid == -1) {
        log_log(LOG_ERROR, "Failed to fork child process");
        return false;
    }

    if (pid == 0) {
        if (delay != 0) {
            for (int i = delay; i != 0; --i) {
                log_log(LOG_INFO, "Executing command in %d seconds", i);
                sleep(1);
            }
        }

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

static inline Vector2 vector2_clamp(Vector2 v, Vector2 min, Vector2 max)
{
    return (Vector2) {
        .x = fminf(max.x, fmaxf(min.x, v.x)),
        .y = fminf(max.y, fmaxf(min.y, v.y)),
    };
}

struct {
    Server* server;
    Renderer* renderer;

    Vector2 cursor_position;
    Vector2 minimum_screen_size;
} APP;

bool application_init(int argc, char const** argv)
{
    memset(&APP, 0, sizeof(APP));

    // Get real UID and GID
    uid_t real_uid = getuid();
    gid_t real_gid = getgid();

    // Setup listening for mouse/keyboard events
    input_start_processing();

    // Set effective UID and GID to the real ones
    if (seteuid(real_uid) == -1) {
        log_log(LOG_WARNING, "Could not set the effective UID to the real one: %s\n"
                             "If the server was started as root, it will keep running as root. "
                             "Please be cautious!",
                strerror(errno));
    }

    if (setegid(real_gid) == -1) {
        log_log(LOG_WARNING, "Could not set the effective UID to the real one: %s\n"
                             "If the server was started as root, it will keep running as root. "
                             "Please be cautious!",
                strerror(errno));
    }

    // Proceeed with server initialization

    char const* command = NULL;
    if (argc > 1)
        command = argv[1];

    if (!glext_load_extensions()) {
        return false;
    }

    log_log(LOG_INFO, "Initializing WindowRenderer");
    session_init();
    log_log(LOG_INFO, "Session hash: %s", session_get_hash());

    wm_init();

    APP.server = server_create();
    if (!server_run(APP.server))
        return false;

    if (!execute_command(1, (char const*[]) { command }, 2))
        return false;

    return true;
}

void application_terminate()
{
    server_destroy(APP.server);
}

bool application_init_graphics(int width, int height)
{
    APP.minimum_screen_size = (Vector2) { width, height };

    APP.renderer = renderer_create(width, height);
    if (!APP.renderer)
        return false;
    return true;
}

void application_destroy_graphics()
{
    renderer_destroy(APP.renderer);
}

void application_resize(int width, int height)
{
    if (width < APP.minimum_screen_size.x)
        APP.minimum_screen_size.x = width;
    if (width < APP.minimum_screen_size.y)
        APP.minimum_screen_size.y = height;

    renderer_resize(APP.renderer, width, height);
}

static void draw_window(EGLDisplay* egl_display, Window* window)
{
    WMWindowParameters window_parameters = wm_compute_window_parameters(window);

    // Draw window border
    renderer_draw_rectangle(APP.renderer,
                            window_parameters.border_position,
                            window_parameters.border_size,
                            (Vector4) { 0.0f, 0.0f, 1.0f, 1.0f });

    // Draw title bar
    renderer_draw_rectangle(APP.renderer,
                            window_parameters.title_bar_position,
                            window_parameters.title_bar_size,
                            (Vector4) { 1.0f, 1.0f, 0.0f, 1.0f });

    // Draw window content
    {
        Vector2 window_content_size = { window->width, window->height };

        renderer_draw_rectangle(APP.renderer,
                                window_parameters.content_position,
                                window_content_size,
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
                return;
            }

            Texture* texture = texture_create_from_egl_imagekhr(egl_image,
                                                                window->dma_buf.width,
                                                                window->dma_buf.height);

            renderer_draw_texture_ex(APP.renderer,
                                     texture,
                                     window_parameters.content_position,
                                     window_content_size,
                                     (Vector4) { 1.0f, 1.0f, 1.0f, 1.0f });

            texture_destroy(texture);

            eglDestroyImageKHR(egl_display, egl_image);
        }
    }
}

void application_render(EGLDisplay* egl_display)
{
    (void)egl_display;

    gl(ClearColor, 0.8f, 0.8f, 0.8f, 1.0f);
    gl(Clear, GL_COLOR_BUFFER_BIT);

    renderer_begin_drawing(APP.renderer);

    server_lock_windows(APP.server);

    for (size_t i = 0; i < server_get_window_count(APP.server); ++i) {
        Window* window = server_get_windows(APP.server)[i];
        draw_window(egl_display, window);
    }

    renderer_draw_rectangle(APP.renderer,
                            APP.cursor_position, (Vector2) { 5, 5 },
                            (Vector4) { 0.0f, 1.0f, 0.0f, 1.0f });

    server_unlock_windows(APP.server);
}

void application_update()
{
    wm_update(APP.server, APP.cursor_position);

    Vector2 mouse_delta = get_mouse_delta();
    APP.cursor_position = vector2_clamp((Vector2) {
                                            .x = APP.cursor_position.x + mouse_delta.x,
                                            .y = APP.cursor_position.y + mouse_delta.y,
                                        },
                                        (Vector2) { 0, 0 }, APP.minimum_screen_size);
    input_update();
}

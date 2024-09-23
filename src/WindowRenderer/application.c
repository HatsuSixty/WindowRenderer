#include "application.h"

#include "renderer/opengl/gl_errors.h"
#include "renderer/renderer.h"
#include "session.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <EGL/egl.h>

Application* application_create(void)
{
    Application* application = malloc(sizeof(*application));
    memset(application, 0, sizeof(*application));

    printf("Initializing WindowRenderer\n");

    session_init();

    printf("Session hash: %s\n", session_get_hash());

    application->server = server_create();
    if (!server_run(application->server))
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

        // Texture* texture = texture_create(window->pixels, window->width, window->height);

        // renderer_draw_texture(application->renderer,
        //                       texture,
        //                       (Vector2) { 0, 0 },
        //                       (Vector4) { 1.0f, 1.0f, 1.0f, 1.0f });

        // texture_destroy(texture);
    }

    server_unlock_windows(application->server);
}

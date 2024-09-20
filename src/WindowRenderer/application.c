#include "application.h"

#include "renderer/opengl/gl_errors.h"

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

Application* application_create(void)
{
    Application* application = malloc(sizeof(*application));
    memset(application, 0, sizeof(*application));

    application->server = server_create();

    return application;
}

void application_destroy(Application* application)
{
    server_destroy(application->server);
    free(application);
}

bool application_init_graphics(Application* application)
{
    application->renderer = renderer_create();
    if (!application->renderer) return false;
    return true;
}

void application_destroy_graphics(Application* application)
{
    renderer_destroy(application->renderer);
}

void application_render(Application* application)
{
    (void)application;

    gl(ClearColor, 0.8f, 0.8f, 0.8f, 1.0f);
    gl(Clear, GL_COLOR_BUFFER_BIT);

    renderer_begin_drawing(application->renderer);

    renderer_draw_triangle(application->renderer,
                           (Vector2){-0.5f, -0.5f},
                           (Vector2){+0.5f, -0.5f},
                           (Vector2){+0.5f, +0.5f},
                           (Vector4){1.0f, 0.0f, 0.0f, 1.0f});
}
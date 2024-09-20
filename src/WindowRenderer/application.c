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
    application->vertex_array = vertex_array_create();

    application->vertex_buffer =
        vertex_array_bind_vertex_buffer(application->vertex_array);
    application->index_buffer =
        vertex_array_bind_index_buffer(application->vertex_array);

    vertex_array_unbind_all(application->vertex_array);

    const char* vertex_shader_source =
        "#version 100\n"
        ""
        "attribute vec4 a_position;\n"
        "attribute vec2 a_tex_coord;\n"
        "attribute vec4 a_color;\n"
        ""
        "varying vec2 v_tex_coord;\n"
        "varying vec4 v_color;\n"
        ""
        "void main() {\n"
        "    gl_Position = a_position;\n"
        "    v_tex_coord = a_tex_coord;\n"
        "    v_color = a_color;\n"
        "}";

    const char* fragment_shader_source =
        "#version 100\n"
        ""
        "precision mediump float;\n"
        ""
        "uniform sampler2D u_texture_slot;\n"
        ""
        "varying vec2 v_tex_coord;\n"
        "varying vec4 v_color;\n"
        ""
        "void main() {\n"
        "    vec4 tex_color = texture2D(u_texture_slot, v_tex_coord);\n"
        "    gl_FragColor = tex_color * v_color;\n"
        "}";

    application->shader = shader_create(vertex_shader_source, fragment_shader_source);
    if (!application->shader->valid) {
        return false;
    }

    unsigned char pixels[] = { 0xFF, 0xFF, 0xFF, 0xFF };
    application->default_texture = texture_create(pixels, 1, 1);

    return true;
}

void application_destroy_graphics(Application* application)
{
    vertex_array_destroy(application->vertex_array);
    shader_destroy(application->shader);
    texture_destroy(application->default_texture);
}

void application_render(Application* application)
{
    (void)application;
    
    gl(ClearColor, 0.8f, 0.8f, 0.8f, 1.0f);
    gl(Clear, GL_COLOR_BUFFER_BIT);

    vertex_array_bind(application->vertex_array);
    texture_bind(application->default_texture, 0);
    shader_bind(application->shader);
    shader_set_uniform_1i(application->shader, "u_texture_slot", 0);

    vertex_buffer_push_vertex(application->vertex_buffer, (Vertex) {
        .position_x = -0.5f, .position_y = -0.5f,
        .texcoord_x = 0.0f, .texcoord_y = 1.0f,
        .color_r = 1.0f, .color_g = 0.0f, .color_b = 0.0f, .color_a = 1.0f,
    });
    vertex_buffer_push_vertex(application->vertex_buffer, (Vertex) {
        .position_x = 0.0f, .position_y = 0.5f,
        .texcoord_x = 0.5f, .texcoord_y = 0.0f,
        .color_r = 0.0f, .color_g = 0.0f, .color_b = 1.0f, .color_a = 1.0f,
    });
    vertex_buffer_push_vertex(application->vertex_buffer, (Vertex) {
        .position_x = 0.5f, .position_y = -0.5f,
        .texcoord_x = 1.0f, .texcoord_y = 1.0f,
        .color_r = 0.0f, .color_g = 1.0f, .color_b = 0.0f, .color_a = 1.0f,
    });

    index_buffer_push_index(application->index_buffer, 0);
    index_buffer_push_index(application->index_buffer, 1);
    index_buffer_push_index(application->index_buffer, 2);

    gl(DrawElements, GL_TRIANGLES, index_buffer_count(application->index_buffer),
                     GL_UNSIGNED_INT, NULL);

    vertex_buffer_clear(application->vertex_buffer);
    index_buffer_clear(application->index_buffer);
}
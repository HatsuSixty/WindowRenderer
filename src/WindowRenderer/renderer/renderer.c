#include "renderer.h"

#include <stdlib.h>
#include <string.h>

#include "opengl/gl_errors.h"

// static void renderer_bind_texture(Renderer* renderer, Texture* texture)
// {
//     texture_bind(texture, 0);
//     shader_bind(renderer->default_shader);
//     shader_set_uniform_1i(renderer->default_shader, "u_texture_slot", 0);
// }

static void sort_triangle(Vector2* a, Vector2* b, Vector2* c)
{
    float area =
        (b->x - a->x) * (c->y - a->y) - (b->y - a->y) * (c->x - a->x);
    if (area < 0.0f) {
        Vector2 temp = *b;
        *b = *c;
        *c = temp;
    }
}

static void renderer_clear_buffers(Renderer* renderer)
{
    vertex_buffer_clear(renderer->vertex_buffer);
    index_buffer_clear(renderer->index_buffer);
}

Renderer* renderer_create()
{
    Renderer* renderer = malloc(sizeof(*renderer));
    memset(renderer, 0, sizeof(*renderer));

    renderer->vertex_array = vertex_array_create();

    renderer->vertex_buffer =
        vertex_array_bind_vertex_buffer(renderer->vertex_array);
    renderer->index_buffer =
        vertex_array_bind_index_buffer(renderer->vertex_array);

    vertex_array_unbind_all(renderer->vertex_array);

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

    renderer->default_shader =
        shader_create(vertex_shader_source, fragment_shader_source);
    if (!renderer->default_shader) {
        return NULL;
    }

    unsigned char pixels[] = { 0xFF, 0xFF, 0xFF, 0xFF };
    renderer->default_texture = texture_create(pixels, 1, 1);

    return renderer;
}

void renderer_destroy(Renderer* renderer)
{
    vertex_array_destroy(renderer->vertex_array);
    shader_destroy(renderer->default_shader);
    texture_destroy(renderer->default_texture);
    free(renderer);
}

void renderer_begin_drawing(Renderer* renderer)
{
    vertex_array_bind(renderer->vertex_array);
    texture_bind(renderer->default_texture, 0);
    shader_bind(renderer->default_shader);
    shader_set_uniform_1i(renderer->default_shader, "u_texture_slot", 0);
}

void renderer_draw_triangle(Renderer* renderer,
                            Vector2 a, Vector2 b, Vector2 c,
                            Vector4 color)
{
    sort_triangle(&a, &b, &c);

    vertex_buffer_push_vertex(renderer->vertex_buffer, (Vertex) {
        V2X(a), 0.0f, 0.0f, V4X(color),
    });
    vertex_buffer_push_vertex(renderer->vertex_buffer, (Vertex) {
        V2X(b), 1.0f, 0.0f, V4X(color),
    });
    vertex_buffer_push_vertex(renderer->vertex_buffer, (Vertex) {
        V2X(c), 1.0f, 1.0f, V4X(color),
    });

    index_buffer_push_index(renderer->index_buffer, 0);
    index_buffer_push_index(renderer->index_buffer, 1);
    index_buffer_push_index(renderer->index_buffer, 2);

    gl(DrawElements, GL_TRIANGLES, index_buffer_count(renderer->index_buffer),
                     GL_UNSIGNED_INT, NULL);

    renderer_clear_buffers(renderer);
}

void renderer_draw_texture(Renderer* renderer, Texture* texture,
                           Vector2 position, Vector4 tint)
{
    renderer_clear_buffers(renderer);
}

void renderer_draw_texture_ex(Renderer* renderer, Texture* texture,
                              Vector2 position, Vector2 size, Vector4 tint)
{
    renderer_clear_buffers(renderer);
}

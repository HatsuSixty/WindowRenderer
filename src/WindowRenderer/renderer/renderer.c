#include "renderer.h"

#include <stdlib.h>
#include <string.h>

#include "opengl/gl_errors.h"

typedef struct {
    float m0, m4, m8, m12;
    float m1, m5, m9, m13;
    float m2, m6, m10, m14;
    float m3, m7, m11, m15;
} Matrix;

static Matrix make_orthogonal_matrix(float min_x, float max_x, float min_y, float max_y)
{
    float near = -1.0f;
    float far = 1.0f;

    Matrix mat = { 0 };

    mat.m0 = 2.0f / (max_x - min_x);
    mat.m1 = 0.0f;
    mat.m2 = 0.0f;
    mat.m3 = 0.0f;

    mat.m4 = 0.0f;
    mat.m5 = 2.0f / (min_y - max_y);
    mat.m6 = 0.0f;
    mat.m7 = 0.0f;

    mat.m8 = 0.0f;
    mat.m9 = 0.0f;
    mat.m10 = -2.0f / (far - near);
    mat.m11 = 0.0f;

    mat.m12 = -(max_x + min_x) / (max_x - min_x);
    mat.m13 = -(max_y + min_y) / (min_y - max_y);
    mat.m14 = -(far + near) / (far - near);
    mat.m15 = 1.0f;

    return mat;
}

static void sort_triangle(Vector2* a, Vector2* b, Vector2* c)
{
    float area = (b->x - a->x) * (c->y - a->y) - (b->y - a->y) * (c->x - a->x);
    if (area < 0.0f) {
        Vector2 temp = *b;
        *b = *c;
        *c = temp;
    }
}

static void renderer_bind_texture(Renderer* renderer, Texture* texture)
{
    texture_bind(texture, 0);
    shader_set_uniform_1i(renderer->default_shader, "u_texture_slot", 0);
}

static void renderer_clear_buffers(Renderer* renderer)
{
    vertex_buffer_clear(renderer->vertex_buffer);
    index_buffer_clear(renderer->index_buffer);
}

Renderer* renderer_create(int width, int height)
{
    Renderer* renderer = malloc(sizeof(*renderer));
    memset(renderer, 0, sizeof(*renderer));

    gl(Viewport, 0, 0, width, height);

    renderer->screen_width = width;
    renderer->screen_height = height;

    renderer->vertex_array = vertex_array_create();

    renderer->vertex_buffer = vertex_array_bind_vertex_buffer(renderer->vertex_array);
    renderer->index_buffer = vertex_array_bind_index_buffer(renderer->vertex_array);

    vertex_array_unbind_all(renderer->vertex_array);

    const char* vertex_shader_source = "#version 100\n"
                                       ""
                                       "attribute vec4 a_position;\n"
                                       "attribute vec2 a_tex_coord;\n"
                                       "attribute vec4 a_color;\n"
                                       ""
                                       "varying vec2 v_tex_coord;\n"
                                       "varying vec4 v_color;\n"
                                       ""
                                       "uniform mat4 u_MVP;\n"
                                       ""
                                       "void main()\n"
                                       "{\n"
                                       "    gl_Position = a_position * u_MVP;\n"
                                       "    v_tex_coord = a_tex_coord;\n"
                                       "    v_color = a_color;\n"
                                       "}";

    const char* fragment_shader_source = "#version 100\n"
                                         ""
                                         "precision mediump float;\n"
                                         ""
                                         "uniform sampler2D u_texture_slot;\n"
                                         ""
                                         "varying vec2 v_tex_coord;\n"
                                         "varying vec4 v_color;\n"
                                         ""
                                         "void main()\n"
                                         "{\n"
                                         "    vec4 tex_color = texture2D(u_texture_slot, v_tex_coord);\n"
                                         "    gl_FragColor = tex_color * v_color;\n"
                                         "}";

    renderer->default_shader = shader_create(vertex_shader_source, fragment_shader_source);
    if (!renderer->default_shader) {
        return NULL;
    }

    shader_bind(renderer->default_shader);

    Matrix mvp_mat = make_orthogonal_matrix(0, width, 0, height);

    shader_set_uniform_mat4x4f(renderer->default_shader, "u_MVP",
                               mvp_mat.m0, mvp_mat.m4, mvp_mat.m8, mvp_mat.m12,
                               mvp_mat.m1, mvp_mat.m5, mvp_mat.m9, mvp_mat.m13,
                               mvp_mat.m2, mvp_mat.m6, mvp_mat.m10, mvp_mat.m14,
                               mvp_mat.m3, mvp_mat.m7, mvp_mat.m11, mvp_mat.m15);

    shader_unbind(renderer->default_shader);

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
    shader_bind(renderer->default_shader);
    renderer_bind_texture(renderer, renderer->default_texture);
}

void renderer_resize(Renderer* renderer, int width, int height)
{
    gl(Viewport, 0, 0, width, height);
    renderer->screen_width = width;
    renderer->screen_height = height;
}

void renderer_draw_triangle(Renderer* renderer,
                            Vector2 a, Vector2 b, Vector2 c,
                            Vector4 color)
{
    sort_triangle(&a, &b, &c);

    vertex_buffer_push_vertex(renderer->vertex_buffer, (Vertex) {
                                                           V2X(a),
                                                           0.0f,
                                                           0.0f,
                                                           V4X(color),
                                                       });
    vertex_buffer_push_vertex(renderer->vertex_buffer, (Vertex) {
                                                           V2X(b),
                                                           1.0f,
                                                           0.0f,
                                                           V4X(color),
                                                       });
    vertex_buffer_push_vertex(renderer->vertex_buffer, (Vertex) {
                                                           V2X(c),
                                                           1.0f,
                                                           1.0f,
                                                           V4X(color),
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
    renderer_draw_texture_ex(renderer, texture, position,
                             (Vector2) { texture->width, texture->height },
                             tint);
}

void renderer_draw_texture_ex(Renderer* renderer, Texture* texture,
                              Vector2 position, Vector2 size, Vector4 tint)
{
    Vector2 a = { position.x, position.y + size.y };
    Vector2 b = { position.x + size.x, position.y + size.y };
    Vector2 c = { position.x + size.x, position.y };
    Vector2 d = position;

    renderer_bind_texture(renderer, texture);

    vertex_buffer_push_vertex(renderer->vertex_buffer, (Vertex) {
                                                           V2X(a),
                                                           0.0f,
                                                           0.0f,
                                                           V4X(tint),
                                                       });
    vertex_buffer_push_vertex(renderer->vertex_buffer, (Vertex) {
                                                           V2X(b),
                                                           1.0f,
                                                           0.0f,
                                                           V4X(tint),
                                                       });
    vertex_buffer_push_vertex(renderer->vertex_buffer, (Vertex) {
                                                           V2X(c),
                                                           1.0f,
                                                           1.0f,
                                                           V4X(tint),
                                                       });
    vertex_buffer_push_vertex(renderer->vertex_buffer, (Vertex) {
                                                           V2X(d),
                                                           0.0f,
                                                           1.0f,
                                                           V4X(tint),
                                                       });

    // First triangle
    index_buffer_push_index(renderer->index_buffer, 0);
    index_buffer_push_index(renderer->index_buffer, 1);
    index_buffer_push_index(renderer->index_buffer, 2);

    // Second triangle
    index_buffer_push_index(renderer->index_buffer, 2);
    index_buffer_push_index(renderer->index_buffer, 3);
    index_buffer_push_index(renderer->index_buffer, 0);

    gl(DrawElements, GL_TRIANGLES, index_buffer_count(renderer->index_buffer),
       GL_UNSIGNED_INT, NULL);

    renderer_bind_texture(renderer, renderer->default_texture);
    renderer_clear_buffers(renderer);
}

void renderer_draw_rectangle(Renderer* renderer,
                             Vector2 position, Vector2 size, Vector4 color)
{
    Vector2 a = { position.x, position.y + size.y };
    Vector2 b = { position.x + size.x, position.y + size.y };
    Vector2 c = { position.x + size.x, position.y };
    Vector2 d = position;

    vertex_buffer_push_vertex(renderer->vertex_buffer, (Vertex) {
                                                           V2X(a),
                                                           0.0f,
                                                           0.0f,
                                                           V4X(color),
                                                       });
    vertex_buffer_push_vertex(renderer->vertex_buffer, (Vertex) {
                                                           V2X(b),
                                                           1.0f,
                                                           0.0f,
                                                           V4X(color),
                                                       });
    vertex_buffer_push_vertex(renderer->vertex_buffer, (Vertex) {
                                                           V2X(c),
                                                           1.0f,
                                                           1.0f,
                                                           V4X(color),
                                                       });
    vertex_buffer_push_vertex(renderer->vertex_buffer, (Vertex) {
                                                           V2X(d),
                                                           0.0f,
                                                           1.0f,
                                                           V4X(color),
                                                       });

    // First triangle
    index_buffer_push_index(renderer->index_buffer, 0);
    index_buffer_push_index(renderer->index_buffer, 1);
    index_buffer_push_index(renderer->index_buffer, 2);

    // Second triangle
    index_buffer_push_index(renderer->index_buffer, 2);
    index_buffer_push_index(renderer->index_buffer, 3);
    index_buffer_push_index(renderer->index_buffer, 0);

    gl(DrawElements, GL_TRIANGLES, index_buffer_count(renderer->index_buffer),
       GL_UNSIGNED_INT, NULL);

    renderer_clear_buffers(renderer);
}

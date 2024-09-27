#ifndef RENDERER_H_
#define RENDERER_H_

#include "opengl/index_buffer.h"
#include "opengl/shader.h"
#include "opengl/texture.h"
#include "opengl/vertex_array.h"
#include "opengl/vertex_buffer.h"

#include "types.h"

typedef struct {
    int screen_width;
    int screen_height;

    Shader* default_shader;
    Texture* default_texture;

    VertexArray* vertex_array;
    VertexBuffer* vertex_buffer;
    IndexBuffer* index_buffer;
} Renderer;

// Returns NULL on error
Renderer* renderer_create(int width, int height);
void renderer_destroy(Renderer* renderer);

void renderer_begin_drawing(Renderer* renderer);

void renderer_resize(Renderer* renderer, int width, int height);

void renderer_draw_triangle(Renderer* renderer,
                            Vector2 a, Vector2 b, Vector2 c,
                            Vector4 color);
void renderer_draw_texture(Renderer* renderer, Texture* texture,
                           Vector2 position, Vector4 tint);
void renderer_draw_texture_ex(Renderer* renderer, Texture* texture,
                              Vector2 position, Vector2 size, Vector4 tint);
void renderer_draw_rectangle(Renderer* renderer,
                             Vector2 position, Vector2 size, Vector4 color);

#endif // RENDERER_H_

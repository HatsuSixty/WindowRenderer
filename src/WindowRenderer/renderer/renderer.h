#ifndef RENDERER_H_
#define RENDERER_H_

#include "opengl/vertex_array.h"
#include "opengl/vertex_buffer.h"
#include "opengl/index_buffer.h"
#include "opengl/texture.h"
#include "opengl/shader.h"

typedef struct {
    float x;
    float y;
} Vector2;
#define V2X(vec) vec.x, vec.y

typedef struct {
    float x;
    float y;
    float z;
    float w;
} Vector4;
#define V4X(vec) vec.x, vec.y, vec.z, vec.w

typedef struct {
    Shader* default_shader;
    Texture* default_texture;
    
    VertexArray* vertex_array;
    VertexBuffer* vertex_buffer;
    IndexBuffer* index_buffer;
} Renderer;

Renderer* renderer_create();
void renderer_destroy(Renderer* renderer);

void renderer_begin_drawing(Renderer* renderer);

void renderer_draw_triangle(Renderer* renderer,
                            Vector2 a, Vector2 b, Vector2 c,
                            Vector4 color);
void renderer_draw_texture(Renderer* renderer, Texture* texture, 
                           Vector2 position, Vector4 tint);
void renderer_draw_texture_ex(Renderer* renderer, Texture* texture,
                              Vector2 position, Vector2 size, Vector4 tint);

#endif // RENDERER_H_
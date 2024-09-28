#pragma once

#include <stddef.h>

#include <GLES2/gl2.h>

typedef struct __attribute__((__packed__)) {
    float position_x, position_y;
    float texcoord_x, texcoord_y;
    float color_r, color_g, color_b, color_a;
} Vertex;

typedef struct {
    GLuint id;
    size_t size;
    size_t capacity;
    unsigned char* cpu_data;
} VertexBuffer;

VertexBuffer* vertex_buffer_bind_new();
void vertex_buffer_destroy(VertexBuffer* vb);

void vertex_buffer_setup_layout(VertexBuffer* vb);

void vertex_buffer_bind(VertexBuffer* vb);
void vertex_buffer_unbind(VertexBuffer* vb);

void vertex_buffer_clear(VertexBuffer* vb);
void vertex_buffer_resize(VertexBuffer* vb, size_t added_size);
void vertex_buffer_push_vertex(VertexBuffer* vb, Vertex vertex);

size_t vertex_buffer_count(VertexBuffer* vb);

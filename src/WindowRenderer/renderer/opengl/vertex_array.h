#ifndef VERTEX_ARRAY_H_
#define VERTEX_ARRAY_H_

#include <GLES2/gl2.h>

#include "vertex_buffer.h"
#include "index_buffer.h"

typedef struct {
    GLuint id;

    VertexBuffer* vertex_buffer;
    IndexBuffer* index_buffer;
} VertexArray;

VertexArray* vertex_array_create(void);
void vertex_array_destroy(VertexArray* va);

void vertex_array_bind(VertexArray* va);
void vertex_array_unbind(VertexArray* va);
void vertex_array_unbind_all(VertexArray* va);

VertexBuffer* vertex_array_bind_vertex_buffer(VertexArray* va);
IndexBuffer* vertex_array_bind_index_buffer(VertexArray* va);

#endif // VERTEX_ARRAY_H_
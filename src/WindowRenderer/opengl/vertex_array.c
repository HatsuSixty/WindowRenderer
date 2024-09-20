#include "vertex_array.h"

#include <stdlib.h>
#include <stddef.h>
#include <string.h>

#include "gl_errors.h"

VertexArray* vertex_array_create(void)
{
    VertexArray* vertex_array = malloc(sizeof(*vertex_array));
    memset(vertex_array, 0, sizeof(*vertex_array));

    return vertex_array;
}

void vertex_array_destroy(VertexArray* va)
{
    if (va->vertex_buffer) vertex_buffer_destroy(va->vertex_buffer);
    if (va->index_buffer) index_buffer_destroy(va->index_buffer);
    free(va);
}

void vertex_array_bind(VertexArray* va)
{
    if (va->vertex_buffer) vertex_buffer_bind(va->vertex_buffer);
    if (va->index_buffer) index_buffer_bind(va->index_buffer);
}

void vertex_array_unbind(VertexArray* va)
{
    (void)va;
}

void vertex_array_unbind_all(VertexArray* va)
{
    if (va->vertex_buffer) vertex_buffer_unbind(va->vertex_buffer);
    if (va->index_buffer) index_buffer_unbind(va->index_buffer);
}

VertexBuffer* vertex_array_bind_vertex_buffer(VertexArray* va)
{
    va->vertex_buffer = vertex_buffer_bind_new();
    vertex_buffer_setup_layout(va->vertex_buffer);
    return va->vertex_buffer;
}

IndexBuffer* vertex_array_bind_index_buffer(VertexArray* va)
{
    va->index_buffer = index_buffer_bind_new();
    return va->index_buffer;
}

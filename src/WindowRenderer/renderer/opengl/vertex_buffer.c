#include "vertex_buffer.h"

#include <assert.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>

#include "gl_errors.h"

VertexBuffer* vertex_buffer_bind_new()
{
    VertexBuffer* vertex_buffer = malloc(sizeof(*vertex_buffer));
    memset(vertex_buffer, 0, sizeof(*vertex_buffer));

    gl(GenBuffers, 1, &vertex_buffer->id);

    vertex_buffer_bind(vertex_buffer);
    vertex_buffer_setup_layout(vertex_buffer);

    return vertex_buffer;
}

void vertex_buffer_destroy(VertexBuffer* vb)
{
    if (vb->cpu_data)
        free(vb->cpu_data);
    gl(DeleteBuffers, 1, &vb->id);
    free(vb);
}

void vertex_buffer_setup_layout(VertexBuffer* vb)
{
    (void)vb;

    gl(EnableVertexAttribArray, 0);
    gl(VertexAttribPointer, 0, 2, GL_FLOAT, GL_FALSE,
       sizeof(Vertex), (void*)offsetof(Vertex, position_x));

    gl(EnableVertexAttribArray, 1);
    gl(VertexAttribPointer, 1, 2, GL_FLOAT, GL_FALSE,
       sizeof(Vertex), (void*)offsetof(Vertex, texcoord_x));

    gl(EnableVertexAttribArray, 2);
    gl(VertexAttribPointer, 2, 4, GL_FLOAT, GL_FALSE,
       sizeof(Vertex), (void*)offsetof(Vertex, color_r));
}

void vertex_buffer_bind(VertexBuffer* vb)
{
    gl(BindBuffer, GL_ARRAY_BUFFER, vb->id);
}

void vertex_buffer_unbind(VertexBuffer* vb)
{
    (void)vb;
    gl(BindBuffer, GL_ARRAY_BUFFER, 0);
}

void vertex_buffer_clear(VertexBuffer* vb)
{
    vb->size = 0;
}

void vertex_buffer_resize(VertexBuffer* vb, size_t added_size)
{
    if (added_size == 0)
        return;

    size_t new_size = vb->capacity + added_size;

    vb->cpu_data = realloc(vb->cpu_data, new_size);
    memset(vb->cpu_data + vb->capacity, 0, added_size);

    gl(BufferData, GL_ARRAY_BUFFER, new_size, vb->cpu_data, GL_DYNAMIC_DRAW);

    vb->capacity = new_size;
}

void vertex_buffer_push_vertex(VertexBuffer* vb, Vertex vertex)
{
    if ((vb->size + sizeof(Vertex)) > vb->capacity) {
        vertex_buffer_resize(vb, sizeof(Vertex));
    }

    gl(BufferSubData, GL_ARRAY_BUFFER, vb->size, sizeof(Vertex), &vertex);
    *(Vertex*)(vb->cpu_data + vb->size) = vertex;

    vb->size += sizeof(Vertex);
}

size_t vertex_buffer_count(VertexBuffer* vb)
{
    return vb->size / sizeof(Vertex);
}
#include "index_buffer.h"

#include <string.h>
#include <assert.h>
#include <stdlib.h>
#include <stdbool.h>

#include "gl_errors.h"

IndexBuffer* index_buffer_bind_new()
{
    IndexBuffer* index_buffer = malloc(sizeof(*index_buffer));
    memset(index_buffer, 0, sizeof(*index_buffer));

    gl(GenBuffers, 1, &index_buffer->id);

    index_buffer_bind(index_buffer);

    return index_buffer;
}

void index_buffer_destroy(IndexBuffer* ib)
{
    gl(DeleteBuffers, 1, &ib->id);
    if (ib->cpu_data) free(ib->cpu_data);
    free(ib);
}

void index_buffer_bind(IndexBuffer* ib)
{
    gl(BindBuffer, GL_ELEMENT_ARRAY_BUFFER, ib->id);
}

void index_buffer_unbind(IndexBuffer* ib)
{
    (void)ib;
    gl(BindBuffer, GL_ELEMENT_ARRAY_BUFFER, 0);
}

void index_buffer_clear(IndexBuffer* ib)
{
    ib->size = 0;
}

void index_buffer_resize(IndexBuffer* ib, size_t added_byte_size)
{
    if (added_byte_size == 0) return;

    size_t new_size = ib->capacity + added_byte_size;

    ib->cpu_data = realloc(ib->cpu_data, new_size);
    memset(ib->cpu_data + ib->capacity, 0, added_byte_size);

    gl(BufferData, GL_ELEMENT_ARRAY_BUFFER, new_size, ib->cpu_data, GL_DYNAMIC_DRAW);

    ib->capacity = new_size;
}

void index_buffer_push_index(IndexBuffer* ib, unsigned int index)
{
    if ((ib->size + sizeof(unsigned int)) > ib->capacity) {
        index_buffer_resize(ib, sizeof(unsigned int));
    }

    gl(BufferSubData, GL_ELEMENT_ARRAY_BUFFER, ib->size, sizeof(unsigned int), &index);
    *(unsigned int*)(ib->cpu_data + ib->size) = index;

    ib->size += sizeof(unsigned int);
}

size_t index_buffer_count(IndexBuffer* ib)
{
    return ib->size / sizeof(unsigned int);
}

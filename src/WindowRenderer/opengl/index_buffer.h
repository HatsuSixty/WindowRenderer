#ifndef INDEX_BUFFER_H_
#define INDEX_BUFFER_H_

#include <stddef.h>

#include <GLES2/gl2.h>

typedef struct {
    GLuint id;
    size_t size;
    size_t capacity;
    unsigned char* cpu_data;
} IndexBuffer;

IndexBuffer* index_buffer_bind_new();
void index_buffer_destroy(IndexBuffer* ib);

void index_buffer_bind(IndexBuffer* ib);
void index_buffer_unbind(IndexBuffer* ib);

void index_buffer_clear(IndexBuffer* ib);
void index_buffer_resize(IndexBuffer* ib, size_t added_byte_size);
void index_buffer_push_index(IndexBuffer* ib, unsigned int index);

size_t index_buffer_count(IndexBuffer* ib);

#endif // INDEX_BUFFER_H_
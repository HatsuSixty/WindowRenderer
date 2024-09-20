#ifndef WR_APPLICATION_H_
#define WR_APPLICATION_H_

#include "server.h"

#include "opengl/vertex_array.h"
#include "opengl/vertex_buffer.h"
#include "opengl/index_buffer.h"
#include "opengl/shader.h"
#include "opengl/texture.h"

typedef struct {
    Server* server;

    Shader* shader;
    Texture* default_texture;

    VertexArray* vertex_array;
    VertexBuffer* vertex_buffer;
    IndexBuffer* index_buffer;
} Application;

Application* application_create(void);
void application_destroy(Application* application);

bool application_init_graphics(Application* application);
void application_destroy_graphics(Application* application);

void application_render(Application* application);

#endif // WR_APPLICATION_H_
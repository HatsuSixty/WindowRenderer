#ifndef WR_APPLICATION_H_
#define WR_APPLICATION_H_

#include "server.h"

#include "renderer/opengl/vertex_array.h"
#include "renderer/opengl/vertex_buffer.h"
#include "renderer/opengl/index_buffer.h"
#include "renderer/opengl/shader.h"
#include "renderer/opengl/texture.h"

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
#ifndef WR_APPLICATION_H_
#define WR_APPLICATION_H_

#include "server.h"

#include "renderer/renderer.h"

typedef struct {
    Server* server;
    Renderer* renderer;
} Application;

Application* application_create(void);
void application_destroy(Application* application);

bool application_init_graphics(Application* application, int width, int height);
void application_destroy_graphics(Application* application);

void application_resize(Application* application, int width, int height);

void application_render(Application* application);

#endif // WR_APPLICATION_H_
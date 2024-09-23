#ifndef WR_APPLICATION_H_
#define WR_APPLICATION_H_

#include <EGL/egl.h>
#include <EGL/eglext.h>

#include "server.h"
#include "renderer/renderer.h"

typedef struct {
    Server* server;
    Renderer* renderer;
    PFNEGLCREATEIMAGEKHRPROC eglCreateImageKHR;
    PFNEGLDESTROYIMAGEKHRPROC eglDestroyImageKHR;
    PFNGLEGLIMAGETARGETTEXTURE2DOESPROC glEGLImageTargetTexture2DOES;
} Application;

// Returns NULL on error
Application* application_create(int argc, char const** argv);
void application_destroy(Application* application);

// Returns false on error
bool application_init_graphics(Application* application, int width, int height);
void application_destroy_graphics(Application* application);

void application_resize(Application* application, int width, int height);

void application_render(Application* application, EGLDisplay* egl_display);

#endif // WR_APPLICATION_H_

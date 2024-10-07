#pragma once

#include <stdbool.h>

#include <EGL/egl.h>
#include <EGL/eglext.h>

#include "renderer/renderer.h"

bool application_init(int argc, char const** argv);
void application_terminate();

void application_init_graphics(Renderer* renderer);
void application_render(Renderer* renderer, EGLDisplay* egl_display);

void application_update();

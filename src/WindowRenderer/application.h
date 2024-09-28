#pragma once

#include <stdbool.h>

#include <EGL/egl.h>
#include <EGL/eglext.h>

bool application_init(int argc, char const** argv);
void application_terminate();

bool application_init_graphics(int width, int height);
void application_destroy_graphics();

void application_resize(int width, int height);
void application_render(EGLDisplay* egl_display);

#pragma once

#include <stdbool.h>

#include <EGL/egl.h>
#include <EGL/eglext.h>
#include <GLES2/gl2.h>
#include <GLES2/gl2ext.h>

extern PFNEGLGETPLATFORMDISPLAYEXTPROC WRGL_eglGetPlatformDisplayEXT;
extern PFNEGLCREATEIMAGEKHRPROC WRGL_eglCreateImageKHR;
extern PFNEGLDESTROYIMAGEKHRPROC WRGL_eglDestroyImageKHR;
extern PFNGLEGLIMAGETARGETTEXTURE2DOESPROC WRGL_glEGLImageTargetTexture2DOES;

bool glext_load_extensions();

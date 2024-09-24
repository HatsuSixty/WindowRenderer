#include "glext.h"

#include <stdbool.h>
#include <stdio.h>

#include <EGL/egl.h>
#include <EGL/eglext.h>
#include <GLES2/gl2.h>
#include <GLES2/gl2ext.h>

bool WRGL_extensions_loaded = false;

PFNEGLGETPLATFORMDISPLAYEXTPROC WRGL_eglGetPlatformDisplayEXT = NULL;
PFNEGLCREATEIMAGEKHRPROC WRGL_eglCreateImageKHR = NULL;
PFNEGLDESTROYIMAGEKHRPROC WRGL_eglDestroyImageKHR = NULL;
PFNGLEGLIMAGETARGETTEXTURE2DOESPROC WRGL_glEGLImageTargetTexture2DOES = NULL;

#define LOAD_PROC(variable, proc)                                                \
    do {                                                                         \
        variable = (typeof(variable))eglGetProcAddress(#proc);                   \
        if (!variable) {                                                         \
            fprintf(stderr, "ERROR: support for the function `" #proc "` is "    \
                            "required, but the function could not be loaded\n"); \
            return false;                                                        \
        }                                                                        \
    } while (0);

bool glext_load_extensions()
{
    if (WRGL_extensions_loaded)
        return true;

    LOAD_PROC(WRGL_eglGetPlatformDisplayEXT, eglGetPlatformDisplayEXT);
    LOAD_PROC(WRGL_eglCreateImageKHR, eglCreateImageKHR);
    LOAD_PROC(WRGL_eglDestroyImageKHR, eglDestroyImageKHR);
    LOAD_PROC(WRGL_glEGLImageTargetTexture2DOES, glEGLImageTargetTexture2DOES);

    WRGL_extensions_loaded = true;

    return true;
}

#undef LOAD_PROC

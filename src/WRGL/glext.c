#include "glext.h"

#include <stdbool.h>
#include <stdio.h>

#include <EGL/egl.h>
#include <EGL/eglext.h>
#include <GLES2/gl2.h>
#include <GLES2/gl2ext.h>

#include "log.h"

bool extensions_loaded = false;

PFNEGLGETPLATFORMDISPLAYEXTPROC eglGetPlatformDisplayEXT = NULL;
PFNEGLCREATEIMAGEKHRPROC eglCreateImageKHR = NULL;
PFNEGLDESTROYIMAGEKHRPROC eglDestroyImageKHR = NULL;
PFNGLEGLIMAGETARGETTEXTURE2DOESPROC glEGLImageTargetTexture2DOES = NULL;

#define LOAD_PROC(name)                                                           \
    do {                                                                          \
        name = (typeof(name))eglGetProcAddress(#name);                            \
        if (!name) {                                                              \
            log_log(LOG_ERROR, "Support for the function `" #name "` is "         \
                               "required, but the function could not be loaded"); \
            return false;                                                         \
        }                                                                         \
    } while (0);

bool glext_load_extensions()
{
    if (extensions_loaded)
        return true;

    LOAD_PROC(eglGetPlatformDisplayEXT);
    LOAD_PROC(eglCreateImageKHR);
    LOAD_PROC(eglDestroyImageKHR);
    LOAD_PROC(glEGLImageTargetTexture2DOES);

    extensions_loaded = true;

    return true;
}

#undef LOAD_PROC

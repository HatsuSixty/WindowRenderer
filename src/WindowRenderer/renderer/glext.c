#include "glext.h"

#include <stddef.h>
#include <stdio.h>

#include "log.h"

PFNEGLCREATEIMAGEKHRPROC eglCreateImageKHR = NULL;
PFNEGLDESTROYIMAGEKHRPROC eglDestroyImageKHR = NULL;
PFNGLEGLIMAGETARGETTEXTURE2DOESPROC glEGLImageTargetTexture2DOES = NULL;

bool glext_load_extensions()
{
    eglCreateImageKHR
        = (PFNEGLCREATEIMAGEKHRPROC)eglGetProcAddress("eglCreateImageKHR");
    if (!eglCreateImageKHR) {
        log_log(LOG_ERROR, "Support for the EGL function `eglCreateImageKHR` is required, "
                           "but the function could not be loaded");
        return false;
    }

    eglDestroyImageKHR
        = (PFNEGLDESTROYIMAGEKHRPROC)eglGetProcAddress("eglDestroyImageKHR");
    if (!eglDestroyImageKHR) {
        log_log(LOG_ERROR, "Support for the EGL function `eglDestroyImageKHR` is required, "
                           "but the function could not be loaded\n");
        return false;
    }

    glEGLImageTargetTexture2DOES
        = (PFNGLEGLIMAGETARGETTEXTURE2DOESPROC)eglGetProcAddress("glEGLImageTargetTexture2DOES");
    if (!glEGLImageTargetTexture2DOES) {
        log_log(LOG_ERROR, "Support for the OpenGL function `glEGLImageTargetTexture2DOES` "
                           "is required, but the function could not be loaded\n");
        return false;
    }

    return true;
}

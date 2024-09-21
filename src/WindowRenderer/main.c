#ifdef DESKTOP_DEBUG_MODE
    #include "main_glfw.c"
#else
    #include "main_drm.c"
#endif
#include <stdio.h>

#include <GLFW/glfw3.h>

#define GLFW_EXPOSE_NATIVE_EGL
#include <GLFW/glfw3native.h>

#include <EGL/egl.h>

#include "application.h"

int screen_width = 640;
int screen_height = 480;

static void on_window_resize(GLFWwindow* window, int width, int height)
{
    Application* application = glfwGetWindowUserPointer(window);
    application_resize(application, width, height);
}

int main(void)
{
    Application* application = application_create();
    if (!application)
        return 1;

    if (!glfwInit()) {
        fprintf(stderr, "ERROR: failed to initialize GLFW\n");
        return 1;
    }

    glfwWindowHint(GLFW_CLIENT_API, GLFW_OPENGL_ES_API);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 2);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_ANY_PROFILE);

    GLFWwindow* window = glfwCreateWindow(screen_width, screen_height,
                                          "WindowRenderer", NULL, NULL);
    if (!window) {
        fprintf(stderr, "ERROR: failed to create window\n");
        glfwTerminate();
        return -1;
    }
    glfwSetWindowUserPointer(window, application);

    glfwMakeContextCurrent(window);

    EGLDisplay egl_display = glfwGetEGLDisplay();
    if (egl_display == EGL_NO_DISPLAY) {
        fprintf(stderr, "ERROR: failed to get EGL display from GLFW\n");
        glfwTerminate();
        return 1;
    }

    if (!application_init_graphics(application, screen_width, screen_height)) {
        glfwTerminate();
        return 1;
    }

    glfwSetWindowSizeCallback(window, on_window_resize);

    while (!glfwWindowShouldClose(window)) {
        application_render(application, &egl_display);

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    application_destroy_graphics(application);

    glfwTerminate();

    application_destroy(application);

    return 0;
}

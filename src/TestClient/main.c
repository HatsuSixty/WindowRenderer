#include <assert.h>
#include <fcntl.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/mman.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <unistd.h>

#include "LibWR/server.h"
#include "LibWR/window.h"

#include <WRGL/buffer.h>
#include <WRGL/context.h>

#include <GL/gl.h>

int main(void)
{
    int serverfd = server_create();
    if (serverfd == -1)
        return 1;

    Window* window = window_create(serverfd, "Hello, World", 400, 400);
    if (window == NULL) {
        server_destroy(serverfd);
        return 1;
    }

    WRGLBuffer* wrgl_buffer = wrgl_buffer_create_from_window(serverfd, window->id,
                                                             window->width, window->height);
    if (!wrgl_buffer) {
        window_close(window);
        server_destroy(serverfd);
        return 1;
    }

    WRGLContext* wrgl_context = wrgl_context_create_for_buffer(wrgl_buffer);
    if (!wrgl_context) {
        wrgl_buffer_destroy(wrgl_buffer);
        window_close(window);
        server_destroy(serverfd);
    }

    printf("Context created\n");

    const unsigned char* version = glGetString(GL_VERSION);
    printf("[INFO] Using OpenGL version %s\n", version);

    glClearColor(1.0f, 0.0f, 0.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);

    glFlush();

    sleep(10);

    wrgl_context_destroy(wrgl_context);
    wrgl_buffer_destroy(wrgl_buffer);
    window_close(window);
    server_destroy(serverfd);

    return 0;
}

#include <assert.h>
#include <fcntl.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <sys/mman.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <unistd.h>

#include <libwr.h>

#include <WRGL/buffer.h>
#include <WRGL/context.h>

#include <GL/gl.h>

int main(void)
{
    int serverfd = wr_server_connect();
    if (serverfd == -1)
        return 1;

    int width = 400;
    int height = 400;
    int window_id = wr_create_window(serverfd, "Hello, World", width, height);
    if (window_id == -1) {
        wr_server_disconnect(serverfd);
        return 1;
    }

    WRGLBuffer* wrgl_buffer = wrgl_buffer_create_from_window(serverfd, window_id,
                                                             width, height);
    if (!wrgl_buffer) {
        wr_close_window(serverfd, window_id);
        wr_server_disconnect(serverfd);
        return 1;
    }

    WRGLContext* wrgl_context = wrgl_context_create_for_buffer(wrgl_buffer);
    if (!wrgl_context) {
        wrgl_buffer_destroy(wrgl_buffer);
        wr_close_window(serverfd, window_id);
        wr_server_disconnect(serverfd);
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
    wr_close_window(serverfd, window_id);
    wr_server_disconnect(serverfd);

    return 0;
}

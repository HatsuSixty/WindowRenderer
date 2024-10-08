#include <assert.h>
#include <fcntl.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>
#include <sys/mman.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <unistd.h>

#include <WRGL/buffer.h>
#include <WRGL/context.h>
#include <WRGL/wrgl.h>
#include <WindowRenderer/windowrenderer.h>
#include <libwr.h>

#include <GL/gl.h>

#define LOG_IMPLEMENTATION
#include "log.h"

int main(int argc, char const** argv)
{
    int width;
    int height;

    if (argc < 3) {
        width = 400;
        height = 400;
    } else {
        width = atoi(argv[1]);
        height = atoi(argv[2]);
    }

    if (width == 0 || height == 0) {
        log_log(LOG_ERROR, "Invalid window width or height");
        return 1;
    }

    int serverfd = wr_server_connect();
    if (serverfd == -1)
        return 1;

    int window_id = wr_create_window(serverfd, "Hello, World", width, height);
    if (window_id == -1) {
        wr_server_disconnect(serverfd);
        return 1;
    }

    char gpu_device_path[256];
    wrgl_find_gpu_device(gpu_device_path, 256);
    log_log(LOG_INFO, "Using GPU device `%s`", gpu_device_path);

    WRGLBuffer* wrgl_buffer = wrgl_buffer_create_from_window(serverfd, gpu_device_path,
                                                             window_id, width, height);
    if (!wrgl_buffer) {
        wr_close_window(serverfd, window_id);
        wr_server_disconnect(serverfd);
        return 1;
    }

    WRGLContextParameters context_parameters = wrgl_get_default_context_parameters();
    WRGLContext* wrgl_context = wrgl_context_create_for_buffer(wrgl_buffer, context_parameters);
    if (!wrgl_context) {
        wrgl_buffer_destroy(wrgl_buffer);
        wr_close_window(serverfd, window_id);
        wr_server_disconnect(serverfd);
        return 1;
    }

    const unsigned char* version = glGetString(GL_VERSION);
    log_log(LOG_INFO, "Using OpenGL version %s", version);

    glClearColor(1.0f, 0.0f, 0.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);

    glFlush();

    int eventfd = wr_event_connect(window_id);
    if (eventfd == -1) {
        return 1;
    }

    while (true) {
        WindowRendererEvent event;
        if (!wr_event_receive(eventfd, &event)) {
            return 1;
        }

        if (event.kind == WREVENT_CLOSE_WINDOW) {
            log_log(LOG_INFO, "Received close window event", event.kind);
            break;
        }

        if (event.kind == WREVENT_MOUSE_BUTTON) {
            char const* mouse_action
                = event.event.mouse_button.action == WR_MOUSE_BUTTON_ACTION_PRESS
                ? "pressed"
                : "released";

            switch (event.event.mouse_button.kind) {
            case WR_MOUSE_BUTTON_LEFT:
                log_log(LOG_INFO, "Left mouse button %s", mouse_action);
                break;

            case WR_MOUSE_BUTTON_RIGHT:
                log_log(LOG_INFO, "Right mouse button %s", mouse_action);
                break;

            case WR_MOUSE_BUTTON_MIDDLE:
                log_log(LOG_INFO, "Middle mouse button %s", mouse_action);
                break;

            case COUNT_WR_MOUSE_BUTTON:
                break;
            }
        }

        if (event.kind == WREVENT_MOUSE_MOVE) {
            log_log(LOG_INFO, "Mouse moved. Current position: { %d, %d }",
                    event.event.mouse_move.position_x, event.event.mouse_move.position_y);
        }
    }

    if (!wr_event_disconnect(eventfd)) {
        return 1;
    }

    wrgl_context_destroy(wrgl_context);
    wrgl_buffer_destroy(wrgl_buffer);

    if (!wr_close_window(serverfd, window_id)) {
        return 1;
    }

    if (!wr_server_disconnect(serverfd)) {
        return 1;
    }

    return 0;
}

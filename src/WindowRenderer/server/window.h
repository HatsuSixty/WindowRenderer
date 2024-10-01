#pragma once

#include <pthread.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#include "WindowRenderer/windowrenderer.h"

typedef struct {
    bool present;

    int fd;
    int width;
    int height;
    int format;
    int stride;
} WindowDmaBuf;

typedef struct {
    uint32_t id;
    char const* title;
    WindowDmaBuf dma_buf;

    int x;
    int y;
    int width;
    int height;

    pthread_t event_listener_thread;
    int event_socket;
    int client_event_socket;
} Window;

Window* window_create(char const* title, int width, int height);
void window_destroy(Window* window);

// Returns false on error
bool window_send_event(Window* window, WindowRendererEvent event);

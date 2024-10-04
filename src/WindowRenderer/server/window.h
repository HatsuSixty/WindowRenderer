#pragma once

#include <pthread.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#include "WindowRenderer/windowrenderer.h"

#include "event_list.h"

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

    EventList event_list;
    pthread_mutex_t event_list_mutex;

    bool event_listener_thread_running;
    pthread_t event_listener_thread;
    int event_socket;
} Window;

Window* window_create(char const* title, int width, int height);
void window_destroy(Window* window);

void window_send_event(Window* window, WindowRendererEvent event);

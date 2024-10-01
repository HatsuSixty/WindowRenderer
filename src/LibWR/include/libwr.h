#pragma once

#include <stdbool.h>

#include <WindowRenderer/windowrenderer.h>

typedef struct {
    int fd;
    int width;
    int height;
    int format;
    int stride;
} WRDmaBuf;

// Returns -1 on error, otherwise returns serverfd
int wr_server_connect();
bool wr_server_disconnect(int serverfd);

// Returns -1 on error, otherwise returns window_id
int wr_create_window(int serverfd, char const* title, int width, int height);

// Returns false on error
bool wr_close_window(int serverfd, int id);

// Returns false on error
bool wr_set_window_dma_buf(int serverfd, int window_id, WRDmaBuf dma_buf);

// Returns -1 on error, otherwise returns eventfd
int wr_event_connect(int window_id);
bool wr_event_disconnect(int eventfd);

// Returns false on error
bool wr_event_receive(int eventfd, WindowRendererEvent* event);

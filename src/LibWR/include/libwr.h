#ifndef LIBWR_H_
#define LIBWR_H_

#include <stdbool.h>

typedef struct {
    int fd;
    int width;
    int height;
    int format;
    int stride;
} WRDmaBuf;

void wr_log_init(char const* program_name);

// Returns -1 on error, otherwise returns serverfd
int wr_server_connect();
void wr_server_disconnect(int serverfd);

// Returns -1 on error, otherwise returns window_id
int wr_create_window(int serverfd, char const* title, int width, int height);

// Returns false on error
bool wr_close_window(int serverfd, int id);

// Returns false on error
bool wr_set_window_dma_buf(int serverfd, int window_id, WRDmaBuf dma_buf);

#endif // LIBWR_H_

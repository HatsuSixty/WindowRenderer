#ifndef WINDOW_H_
#define WINDOW_H_

#include <stdbool.h>
#include <stddef.h>

typedef struct {
    int id;

    // private state
    int serverfd;
} Window;

// Returns NULL on error
Window* window_create(int serverfd, char const* title, int width, int height);

// Returns false on error
bool window_close(Window* window);

#endif // WINDOW_H_

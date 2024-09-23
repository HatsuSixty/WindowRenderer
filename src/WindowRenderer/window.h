#ifndef WR_WINDOW_H_
#define WR_WINDOW_H_

#include <stdbool.h>
#include <stddef.h>

typedef struct {
    int id;
    int width;
    int height;
    char const* title;
} Window;

Window* window_create(char const* title, int width, int height);

// Returns NULL on error
bool window_destroy(Window* window);

#endif // WR_WINDOW_H_

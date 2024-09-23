#include "window.h"

#include "session.h"

#include <fcntl.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <unistd.h>

Window* window_create(char const* title, int width, int height)
{
    Window* window = malloc(sizeof(*window));
    memset(window, 0, sizeof(*window));

    window->id = session_generate_window_id();
    window->title = title;
    window->width = width;
    window->height = height;

    return window;
}

bool window_destroy(Window* window)
{
    free(window);
    return true;
}

#ifndef WINDOW_RENDERER_H_
#define WINDOW_RENDERER_H_

#include <stdbool.h>

#define SOCKET_PATH "/tmp/windowrenderer.sock"

typedef enum {
    WRERROR_CLOSE_FAILED,
    WRERROR_CREATE_FAILED,
    WRERROR_INVALID_COMMAND,
    WRERROR_INVALID_WINID,
    WRERROR_OK,
} WindowRendererErrorKind;

typedef enum {
    WRRESP_EMPTY,
    WRRESP_WINID,
    WRRESP_ERROR,
} WindowRendererResponseKind;

typedef struct {
    WindowRendererResponseKind kind;
    WindowRendererErrorKind error_kind;
    int window_id;
} WindowRendererResponse;

typedef enum {
    WRCMD_CREATE_WINDOW,
    WRCMD_CLOSE_WINDOW,
} WindowRendererCommandKind;

#define WINDOW_TITLE_SIZE 256

typedef struct {
    WindowRendererCommandKind kind;
    int window_id;

    char window_title[WINDOW_TITLE_SIZE];
    int window_width;
    int window_height;
} WindowRendererCommand;

#endif // WINDOW_RENDERER_H_

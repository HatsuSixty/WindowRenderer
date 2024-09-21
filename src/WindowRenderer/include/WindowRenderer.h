#ifndef WINDOW_RENDERER_H_
#define WINDOW_RENDERER_H_

#include <stdbool.h>

/*
 * In the enviroment variable defined by WR_SESSION_HASH_ENV
 * there should be the session hash of the WindowRenderer instance.
 *
 * The hash can be used to get a window's pixels shared memory, or the
 * server socket path.
 *
 * The format for a window's shared memory name is:
 *
 *    - WRWindow_<hash>_<window id>
 *
 * And the format for a WindowRenderer instance's socket path is:
 *
 *    - /tmp/WindowRenderer_<hash>.sock
 */

#define WR_SESSION_HASH_ENV "WINDOW_RENDERER_SESSION_HASH"

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

#define WR_WINDOW_TITLE_SIZE 256

typedef struct {
    WindowRendererCommandKind kind;
    int window_id;

    char window_title[WR_WINDOW_TITLE_SIZE];
    int window_width;
    int window_height;
} WindowRendererCommand;

#endif // WINDOW_RENDERER_H_

#pragma once

#include "commands/create_window.h"
#include "commands/close_window.h"
#include "commands/set_window_dma_buf.h"

#include "responses/window_id.h"

/*
 * In the enviroment variable defined by WR_SESSION_HASH_ENV
 * there should be the session hash of the WindowRenderer instance.
 *
 * The hash is used to get a WindowRenderer instance's socket path.
 * The format for a WindowRenderer instance's socket path is:
 *
 *    - /tmp/WindowRenderer_<hash>.sock
 */

#define WR_SESSION_HASH_ENV "WINDOW_RENDERER_SESSION_HASH"

// ========================= //

/*                             *
 *   -=-= BEGIN COMMAND =-=-   *
 *                            */

typedef enum {
    WRCMD_CREATE_WINDOW,
    WRCMD_CLOSE_WINDOW,
    WRCMD_SET_WINDOW_DMA_BUF,
} WindowRendererCommandKind;

typedef struct {
    WindowRendererCommandKind kind;

    union {
        WindowRendererCreateWindow create_window;
        WindowRendererCloseWindow close_window;
        WindowRendererSetWindowDmaBuf set_window_dma_buf;
    } command;
} WindowRendererCommand;

/*                           *
 *   -=-= END COMMAND =-=-   *
 *                           */

// ========================= //

/*                              *
 *   -=-= BEGIN RESPONSE =-=-   *
 *                              */

typedef enum {
    WRSTATUS_INVALID_COMMAND,
    WRSTATUS_INVALID_WINID,
    WRSTATUS_INVALID_DMA_BUF_FD,
    WRSTATUS_INVALID_DMA_BUF_SIZE,
    WRSTATUS_OK,
} WindowRendererStatus;

typedef enum {
    WRRESP_WINID,
    WRRESP_EMPTY,
} WindowRendererResponseKind;

typedef struct {
    WindowRendererResponseKind kind;
    WindowRendererStatus status;

    union {
        WindowRendererWindowId window_id;
    } response;
} WindowRendererResponse;

/*                            *
 *   -=-= END RESPONSE =-=-   *
 *                            */

// ========================= //

/*                           *
 *   -=-= BEGIN EVENT =-=-   *
 *                           */

typedef enum {
    WREVENT_CLOSE_WINDOW,
} WindowRendererEventKind;

typedef struct {
    WindowRendererEventKind kind;
} WindowRendererEvent;

/*                         *
 *   -=-= END EVENT =-=-   *
 *                         */

// ========================= //

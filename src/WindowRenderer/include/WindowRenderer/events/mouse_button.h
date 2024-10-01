#pragma once

typedef enum {
    WR_MOUSE_BUTTON_ACTION_PRESS,
    WR_MOUSE_BUTTON_ACTION_RELEASE,
} WindowRendererMouseButtonAction;

typedef enum {
    WR_MOUSE_BUTTON_LEFT = 0,
    WR_MOUSE_BUTTON_RIGHT,
    WR_MOUSE_BUTTON_MIDDLE,
    COUNT_WR_MOUSE_BUTTON,
} WindowRendererMouseButtonKind;

typedef struct {
    WindowRendererMouseButtonKind kind;
    WindowRendererMouseButtonAction action;
} WindowRendererMouseButton;

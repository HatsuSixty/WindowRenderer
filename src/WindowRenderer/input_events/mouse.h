#pragma once

#include <stdbool.h>

typedef enum {
    INPUT_MOUSE_AXIS_X,
    INPUT_MOUSE_AXIS_Y,
} InputMouseAxis;

typedef enum {
    INPUT_MOUSE_BUTTON_LEFT,
    INPUT_MOUSE_BUTTON_RIGHT,
    INPUT_MOUSE_BUTTON_MIDDLE,
    COUNT_INPUT_MOUSE_BUTTON,
} InputMouseButton;

typedef struct {
    void (*button)(InputMouseButton button, bool released, void* user_data);
    void (*move)(InputMouseAxis axis, int units, void* user_data);
    void (*scroll)(int detents, void* user_data);
} InputMouseInterface;

void input_mouse_start_processing(InputMouseInterface interface, void* user_data);

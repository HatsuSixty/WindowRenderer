#include "input.h"

#include <string.h>

#include "input_events/mouse.h"

struct {
    Vector2 mouse_delta;
    bool mouse_buttons[COUNT_INPUT_MOUSE_BUTTON];
    bool prev_mouse_buttons[COUNT_INPUT_MOUSE_BUTTON];
} INPUT;

static void mouse_button(InputMouseButton button, bool released, void* user_data)
{
    (void)user_data;
    INPUT.mouse_buttons[button] = !released;
}

static void mouse_move(InputMouseAxis axis, int units, void* user_data)
{
    (void)user_data;

    switch (axis) {
    case INPUT_MOUSE_AXIS_X:
        INPUT.mouse_delta.x = units;
        break;

    case INPUT_MOUSE_AXIS_Y:
        INPUT.mouse_delta.y = units;
        break;
    }
}

static void mouse_scroll(int detents, void* user_data)
{
    (void)detents;
    (void)user_data;
}

void input_start_processing()
{
    memset(&INPUT, 0, sizeof(INPUT));

    InputMouseInterface mouse_interface = {
        .button = mouse_button,
        .move = mouse_move,
        .scroll = mouse_scroll,
    };
    input_mouse_start_processing(mouse_interface, NULL);
}

void input_update()
{
    INPUT.mouse_delta = (Vector2) { 0, 0 };
    memcpy(INPUT.prev_mouse_buttons, INPUT.mouse_buttons, sizeof(INPUT.mouse_buttons));
}

bool is_mouse_button_just_pressed(InputMouseButton button)
{
    return INPUT.mouse_buttons[button] && !INPUT.prev_mouse_buttons[button];
}

bool is_mouse_button_just_released(InputMouseButton button)
{
    return INPUT.prev_mouse_buttons[button] && !INPUT.mouse_buttons[button];
}

bool is_mouse_button_pressed(InputMouseButton button)
{
    return INPUT.mouse_buttons[button];
}

Vector2 get_mouse_delta()
{
    return INPUT.mouse_delta;
}

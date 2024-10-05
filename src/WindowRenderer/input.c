#include "input.h"

#include <math.h>
#include <string.h>

#include "input_events/mouse.h"

static inline Vector2 vector2_clamp(Vector2 v, Vector2 min, Vector2 max)
{
    return (Vector2) {
        .x = fminf(max.x, fmaxf(min.x, v.x)),
        .y = fminf(max.y, fmaxf(min.y, v.y)),
    };
}
struct {
    Vector2 cursor_bounds;

    Vector2 curr_cursor_position;
    Vector2 next_cursor_position;
    Vector2 prev_cursor_position;

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

    Vector2 next_cursor_position = INPUT.next_cursor_position;

    switch (axis) {
    case INPUT_MOUSE_AXIS_X:
        next_cursor_position.x += units;
        break;

    case INPUT_MOUSE_AXIS_Y:
        next_cursor_position.y += units;
        break;
    }

    INPUT.next_cursor_position = vector2_clamp(next_cursor_position,
                                               (Vector2) { 0, 0 },
                                               INPUT.cursor_bounds);
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
    INPUT.prev_cursor_position = INPUT.curr_cursor_position;
    INPUT.curr_cursor_position = INPUT.next_cursor_position;
    memcpy(INPUT.prev_mouse_buttons, INPUT.mouse_buttons, sizeof(INPUT.mouse_buttons));
}

void input_set_cursor_bounds(Vector2 bounds)
{
    INPUT.cursor_bounds = bounds;
}

Vector2 input_get_cursor_bounds()
{
    return INPUT.cursor_bounds;
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

Vector2 get_cursor_position()
{
    return INPUT.curr_cursor_position;
}

Vector2 get_cursor_delta()
{
    return (Vector2) {
        .x = INPUT.curr_cursor_position.x - INPUT.prev_cursor_position.x,
        .y = INPUT.curr_cursor_position.y - INPUT.prev_cursor_position.y,
    };
}

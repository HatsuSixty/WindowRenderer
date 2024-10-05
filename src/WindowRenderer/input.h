#pragma once

#include <stdbool.h>

#include "input_events/mouse.h"
#include "types.h"

void input_start_processing();
void input_update();

void input_set_cursor_bounds(Vector2 bounds);
Vector2 input_get_cursor_bounds();

bool is_mouse_button_just_pressed(InputMouseButton button);
bool is_mouse_button_just_released(InputMouseButton button);
bool is_mouse_button_pressed(InputMouseButton button);

Vector2 get_cursor_delta();
Vector2 get_cursor_position();

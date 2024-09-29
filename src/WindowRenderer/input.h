#pragma once

#include <stdbool.h>

#include "input_events/mouse.h"
#include "types.h"

void input_start_processing();
void input_update();

bool is_mouse_button_just_pressed(InputMouseButton button);
bool is_mouse_button_just_released(InputMouseButton button);
bool is_mouse_button_pressed(InputMouseButton button);

Vector2 get_mouse_delta();
Vector2 get_mouse_position();

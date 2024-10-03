#pragma once

#include "types.h"

#include "server/window.h"
#include "server/server.h"

typedef struct {
    float border_thickness;
    Vector2 border_position;
    Vector2 border_size;

    float title_bar_thickness;
    Vector2 title_bar_position;
    Vector2 title_bar_size;

    Vector2 close_button_position;
    Vector2 close_button_size;

    Vector2 content_position;

    Vector2 total_area_position;
    Vector2 total_area_size;
} WMWindowParameters;

void wm_init();

WMWindowParameters wm_compute_window_parameters(Window* window);
void wm_update(Server* server,
               Vector2 cursor_position, Vector2 cursor_delta);

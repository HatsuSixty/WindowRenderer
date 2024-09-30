#include "wm.h"

#include <stdint.h>

#include "input.h"
#include "input_events/mouse.h"
#include "server/server.h"
#include "log.h"

static bool check_collision_point_rec(Vector2 point, Vector2 rec_position, Vector2 rec_size)
{
    return ((point.x >= rec_position.x)
            && (point.x < (rec_position.x + rec_size.x))
            && (point.y >= rec_position.y)
            && (point.y < (rec_position.y + rec_size.y)));
}

struct {
    uint32_t dragged_window_id;
    Vector2 drag_offset;
} WM;

void wm_init()
{
    WM.dragged_window_id = -1;
}

WMWindowParameters wm_compute_window_parameters(Window* window)
{
    const float border_thickness = 5.f;
    const float title_bar_thickness = 20.f;

    const Vector2 close_button_position = { window->x, window->y };
    const Vector2 close_button_size = {
        .x = title_bar_thickness,
        .y = title_bar_thickness,
    };

    const Vector2 title_bar_position = {
        .x = close_button_position.x + close_button_size.x,
        .y = close_button_position.y,
    };
    const Vector2 title_bar_size = {
        .x = window->width - close_button_size.x + border_thickness * 2.f,
        .y = title_bar_thickness,
    };

    const Vector2 content_position = {
        .x = window->x + border_thickness,
        .y = window->y + title_bar_thickness,
    };

    const Vector2 border_position = {
        .x = content_position.x - border_thickness,
        .y = content_position.y - border_thickness,
    };
    const Vector2 border_size = {
        .x = window->width + border_thickness * 2.f,
        .y = window->height + border_thickness * 2.f,
    };

    const Vector2 total_area_position = title_bar_position;
    const Vector2 total_area_size = {
        .x = border_size.x,
        .y = border_size.y + title_bar_thickness - border_thickness,
    };

    return (WMWindowParameters) {
        .border_thickness = border_thickness,
        .border_position = border_position,
        .border_size = border_size,

        .title_bar_thickness = title_bar_thickness,
        .title_bar_position = title_bar_position,
        .title_bar_size = title_bar_size,

        .close_button_position = close_button_position,
        .close_button_size = close_button_size,

        .content_position = content_position,

        .total_area_position = total_area_position,
        .total_area_size = total_area_size,
    };
}

void wm_update(Server* server, Vector2 cursor_position)
{
    /*
     * Start updating windows: lock window access
     */
    server_lock_windows(server);

    // Handle window dragging/close button
    {
        for (size_t i = 0; i < server_get_window_count(server); ++i) {
            Window* window = server_get_windows(server)[i];
            bool window_is_active = window->id == server_top_window(server)->id;
            WMWindowParameters window_parameters = wm_compute_window_parameters(window);

            // Handle window dragging
            {
                if (check_collision_point_rec(cursor_position,
                                              window_parameters.title_bar_position,
                                              window_parameters.title_bar_size)
                    && is_mouse_button_just_pressed(INPUT_MOUSE_BUTTON_LEFT)) {
                    WM.dragged_window_id = window->id;
                    WM.drag_offset.x = cursor_position.x - window->x;
                    WM.drag_offset.y = cursor_position.y - window->y;
                }

                if (WM.dragged_window_id == window->id) {
                    if (window_is_active) {
                        window->x = cursor_position.x - WM.drag_offset.x;
                        window->y = cursor_position.y - WM.drag_offset.y;
                    }

                    if (is_mouse_button_just_released(INPUT_MOUSE_BUTTON_LEFT)) {
                        WM.dragged_window_id = -1;
                    }
                }
            }

            // Handle close button
            if (check_collision_point_rec(cursor_position,
                                          window_parameters.close_button_position,
                                          window_parameters.close_button_size)
                && is_mouse_button_just_pressed(INPUT_MOUSE_BUTTON_LEFT)) {
                log_log(LOG_INFO, "Close button of window ID %d clicked!", window->id);
            }
        }
    }

    // Handle window focus
    {
        if (is_mouse_button_just_pressed(INPUT_MOUSE_BUTTON_LEFT)
            && server_get_window_count(server) != 0) {
            for (size_t i = server_get_window_count(server); i-- > 0;) {
                Window* window = server_get_windows(server)[i];
                WMWindowParameters window_parameters = wm_compute_window_parameters(window);

                if (check_collision_point_rec(cursor_position,
                                              window_parameters.total_area_position,
                                              window_parameters.total_area_size)) {
                    if (window->id != server_top_window(server)->id) {
                        server_raise_window(server, window);
                    }
                    break;
                }
            }
        }
    }

    /*
     * Finish updating windows: unlock window access
     */
    server_unlock_windows(server);
}

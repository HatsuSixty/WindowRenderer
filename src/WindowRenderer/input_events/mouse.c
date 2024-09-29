#include "mouse.h"

#include <ctype.h>
#include <dirent.h>
#include <errno.h>
#include <fcntl.h>
#include <linux/input-event-codes.h>
#include <linux/input.h>
#include <pthread.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "log.h"

typedef struct {
    int mouse_fd;
    InputMouseInterface interface;
    void* user_data;
} ProcessMouseEventsInfo;

static void* process_mouse_events(ProcessMouseEventsInfo* info)
{
    int mouse_fd = info->mouse_fd;
    InputMouseInterface mouse_interface = info->interface;
    void* user_data = info->user_data;

    struct input_event event;
    size_t bytes_read;
    while ((bytes_read = read(mouse_fd, &event, sizeof(struct input_event))) > 0) {
        if (bytes_read != sizeof(struct input_event)) {
            log_log(LOG_WARNING, "Incomplete mouse event read");
            continue;
        }

        if (event.type == EV_KEY) {
            InputMouseButton mouse_button;

            switch (event.code) {
            case BTN_LEFT:
                mouse_button = INPUT_MOUSE_BUTTON_LEFT;
                break;

            case BTN_RIGHT:
                mouse_button = INPUT_MOUSE_BUTTON_RIGHT;
                break;

            case BTN_MIDDLE:
                mouse_button = INPUT_MOUSE_BUTTON_MIDDLE;
                break;

            default:
                continue;
            }

            if (mouse_interface.button)
                mouse_interface.button(mouse_button, event.value == 0, user_data);

        } else if (event.type == EV_REL && (event.code == REL_X || event.code == REL_Y)) {
            InputMouseAxis mouse_axis;

            switch (event.code) {
            case REL_X:
                mouse_axis = INPUT_MOUSE_AXIS_X;
                break;

            case REL_Y:
                mouse_axis = INPUT_MOUSE_AXIS_Y;
                break;

            default:
                continue;
            }

            if (mouse_interface.move)
                mouse_interface.move(mouse_axis, event.value, user_data);

        } else if (event.type == EV_ABS && (event.code == ABS_X || event.code == ABS_Y)) {
            InputMouseAxis mouse_axis;

            switch (event.code) {
            case ABS_X:
                mouse_axis = INPUT_MOUSE_AXIS_X;
                break;

            case ABS_Y:
                mouse_axis = INPUT_MOUSE_AXIS_Y;
                break;

            default:
                continue;
            }

            if (mouse_interface.move_abs)
                mouse_interface.move_abs(mouse_axis, event.value, user_data);

        } else if (event.type == EV_REL && event.code == REL_WHEEL) {
            if (mouse_interface.scroll)
                mouse_interface.scroll(event.value, user_data);
        }
    }

    close(mouse_fd);
    free(info);
    return NULL;
}

void input_mouse_start_processing(InputMouseInterface interface, void* user_data)
{
    char const* directory_path = "/dev/input/";

    DIR* directory = opendir(directory_path);
    if (directory == NULL) {
        log_log(LOG_WARNING, "Failed to open directory `%s`: %s\n"
                             "Mouse input won't be available",
                directory_path, strerror(errno));
        return;
    }

    bool mouse_found = false;

    struct dirent* entry;
    while ((entry = readdir(directory)) != NULL) {
        if (strncmp(entry->d_name, "event", 5) == 0) {
            char device_path[256];
            snprintf(device_path, sizeof(device_path), "%s%s", directory_path, entry->d_name);

            int device_fd = open(device_path, O_RDONLY);
            if (device_fd == -1)
                continue;

            char device_name[256];
            if (ioctl(device_fd, EVIOCGNAME(sizeof(device_name)), device_name) == -1) {
                close(device_fd);
                continue;
            }
            for (size_t i = 0; i < 256; ++i)
                device_name[i] = tolower(device_name[i]);

            bool is_mouse_device = strstr(device_name, "mouse") != NULL;

            if (is_mouse_device) {
                ProcessMouseEventsInfo* info = malloc(sizeof(*info));
                memset(info, 0, sizeof(*info));
                info->mouse_fd = device_fd;
                info->interface = interface;
                info->user_data = user_data;

                pthread_t process_mouse_events_thread;
                int status
                    = pthread_create(&process_mouse_events_thread, NULL,
                                     (void* (*)(void*)) & process_mouse_events, info);
                if (status != 0) {
                    log_log(LOG_ERROR, "Could not create `process_mouse_events` thread");
                    continue;
                }
                pthread_detach(process_mouse_events_thread);

                mouse_found = true;

                continue;
            }

            close(device_fd);
        }
    }

    if (!mouse_found) {
        log_log(LOG_WARNING, "No mouse device was found. "
                             "Does WindowRenderer have the right permissions?");
    }

    closedir(directory);
}

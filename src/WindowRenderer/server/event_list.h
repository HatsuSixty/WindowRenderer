#pragma once

#include <stddef.h>

#include "WindowRenderer/windowrenderer.h"

#define EVENT_LIST_MAX 1024

typedef struct {
    WindowRendererEvent events[EVENT_LIST_MAX];
    size_t events_count;
} EventList;

void event_list_push(EventList* event_list, WindowRendererEvent event);
WindowRendererEvent event_list_pop(EventList* event_list);

size_t event_list_get_count(EventList* event_list);

#include "event_list.h"

void event_list_push(EventList* event_list, WindowRendererEvent event)
{
    event_list->events[event_list->events_count % EVENT_LIST_MAX] = event;
    event_list->events_count++;
}

WindowRendererEvent event_list_pop(EventList* event_list)
{
    return event_list->events[--event_list->events_count];
}

size_t event_list_get_count(EventList *event_list)
{
    return event_list->events_count;
}

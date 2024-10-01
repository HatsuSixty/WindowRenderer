#pragma once

#include <stdbool.h>
#include <stdint.h>

bool server_session_init();
char* server_session_get_socket_name();
char* server_session_generate_window_socket_name(uint32_t window_id);

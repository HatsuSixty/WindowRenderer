#pragma once

#include <stdint.h>

void session_init();

char const* session_get_hash();

uint32_t session_generate_window_id();
char* session_generate_socket_name();

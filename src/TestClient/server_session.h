#ifndef SERVER_SESSION_H_
#define SERVER_SESSION_H_

#include <stdbool.h>
#include <stdint.h>

bool server_session_init();
char* server_session_get_window_shm_name(uint32_t window_id);
char* server_session_get_socket_name();

#endif // SERVER_SESSION_H_

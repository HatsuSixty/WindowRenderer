#ifndef SERVER_H_
#define SERVER_H_

#include <stdbool.h>

// Returns -1 on error
int server_create();
void server_destroy(int serverfd);

// Returns -1 on error, otherwise returns window_id
int server_create_window(int serverfd, char const* title, int width, int height);
bool server_close_window(int serverfd, int id);

#endif // SERVER_H_

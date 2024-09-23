#include "server_session.h"

#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "WindowRenderer.h"

#define STRING_STORE_CAP 102400
char string_store[STRING_STORE_CAP] = { 0 };
int string_store_size = 0;

static char* string_store_alloc(size_t count)
{
    char* ptr = &string_store[string_store_size];
    string_store_size += count + 1;
    return ptr;
}

char const* session_hash;
size_t session_hash_length;

bool server_session_init()
{
    session_hash = getenv(WR_SESSION_HASH_ENV);
    if (!session_hash) {
        fprintf(stderr, "ERROR: could not get `%s` environment variable\n", WR_SESSION_HASH_ENV);
        fprintf(stderr, "Is the server running?\n");
        return false;
    }

    session_hash_length = strlen(session_hash);

    return true;
}

char* server_session_get_socket_name()
{
    char const* socket_name_prefix = "/tmp/WindowRenderer_";
    char const* socket_name_postfix = ".sock";

    size_t socket_name_length = strlen(socket_name_prefix)
        + session_hash_length + strlen(socket_name_postfix) + 1;

    char* socket_name = string_store_alloc(socket_name_length);
    snprintf(socket_name, socket_name_length, "%s%s%s",
             socket_name_prefix, session_hash, socket_name_postfix);

    return socket_name;
}

#include "session.h"

#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#define SESSION_HASH_LENGTH 16
char session_hash[SESSION_HASH_LENGTH + 1];

#define STRING_STORE_CAP 102400
char string_store[STRING_STORE_CAP] = { 0 };
int string_store_size = 0;

static char* string_store_alloc(size_t count)
{
    char* ptr = &string_store[string_store_size];
    string_store_size += count + 1;
    return ptr;
}

static unsigned long djb2(const char* str)
{
    unsigned long hash = 5381;
    int c;

    while ((c = *str++)) {
        hash = ((hash << 5) + hash) + c; // hash * 33 + c
    }

    return hash;
}

void session_init()
{
    srand(time(NULL));

    const size_t rand_string_length = 256;
    char* rand_string = alloca(rand_string_length + 1);
    rand_string[rand_string_length] = '\0';
    {
        const char charset[] = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789";
        for (size_t i = 0; i < rand_string_length; i++) {
            int key = rand() % (int)(sizeof(charset) - 1);
            rand_string[i] = charset[key];
        }
    }

    unsigned long hash = djb2(rand_string);
    snprintf(session_hash, SESSION_HASH_LENGTH + 1, "%08lX", hash);

    setenv("WINDOW_RENDERER_SESSION_HASH", session_hash, 1);
}

char const* session_get_hash()
{
    return session_hash;
}

uint32_t window_id_tracker = 0;

uint32_t session_generate_window_id()
{
    uint32_t id = window_id_tracker;
    window_id_tracker++;
    return id;
}

char* session_generate_socket_name()
{
    char const* socket_name_prefix = "/tmp/WindowRenderer_";
    char const* socket_name_postfix = ".sock";

    size_t socket_name_length = strlen(socket_name_prefix)
        + SESSION_HASH_LENGTH + strlen(socket_name_postfix) + 1;

    char* socket_name = string_store_alloc(socket_name_length);
    snprintf(socket_name, socket_name_length, "%s%s%s",
             socket_name_prefix, session_hash, socket_name_postfix);

    return socket_name;
}

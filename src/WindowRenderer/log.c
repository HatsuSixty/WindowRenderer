#include "log.h"

#include <libgen.h>
#include <assert.h>
#include <stdio.h>
#include <stdarg.h>
#include <unistd.h>
#include <string.h>

static void get_process_name(char* restrict buf, size_t len)
{
    char exe_path[256];
    int num_chars = readlink("/proc/self/exe", exe_path, sizeof(exe_path) - 1);
    assert(num_chars != -1);
    exe_path[num_chars] = '\0';

    char* base = basename(exe_path);
    strncpy(buf, base, len - 1);
    buf[len - 1] = '\0';
}

void log_log(LogLevel level, char const* format, ...)
{
    FILE* stream = stdout;

    char program_name[256];
    get_process_name(program_name, 256);

    switch (level) {
    case LOG_WARNING:
        stream = stderr;
        fprintf(stream, "%s: [WARNING] ", program_name);
        break;

    case LOG_INFO:
        fprintf(stream, "%s: [INFO] ", program_name);
        break;

    case LOG_ERROR:
        stream = stderr;
        fprintf(stream, "%s: [ERROR] ", program_name);
        break;
    }

    va_list arg_list;
    va_start(arg_list, format);
    vfprintf(stream, format, arg_list);
    va_end(arg_list);

    fprintf(stream, "\n");
}

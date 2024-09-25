#include "log.h"

#include <stdio.h>
#include <stdarg.h>

LogLevel log_log_level = LOG_WARNING;
char const* log_program_name = NULL;

void log_init(char const* program_name)
{
    log_program_name = program_name;
}

void log_set_level(LogLevel level)
{
    log_log_level = level;
}

void log_log(LogLevel level, char const* format, ...)
{
    if (!(level >= log_log_level))
        return;

    FILE* stream = stdout;

    switch (level) {
    case LOG_WARNING:
        stream = stderr;
        fprintf(stream, "%s: [WARNING] ", log_program_name);
        break;

    case LOG_INFO:
        fprintf(stream, "%s: [INFO] ", log_program_name);
        break;

    case LOG_ERROR:
        stream = stderr;
        fprintf(stream, "%s: [ERROR] ", log_program_name);
        break;

    case LOG_NO_LOG:
        return;
    }

    va_list arg_list;
    va_start(arg_list, format);
    vfprintf(stream, format, arg_list);
    va_end(arg_list);

    fprintf(stream, "\n");
}

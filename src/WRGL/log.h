#ifndef LOG_H_
#define LOG_H_

typedef enum {
    LOG_WARNING,
    LOG_INFO,
    LOG_ERROR,
    LOG_NO_LOG,
} LogLevel;

void log_init(char const* program_name);
void log_set_level(LogLevel level);
void log_log(LogLevel level, char const* format, ...);

#endif // LOG_H_

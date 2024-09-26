#ifndef LOG_H_
#define LOG_H_

typedef enum {
    LOG_WARNING,
    LOG_INFO,
    LOG_ERROR,
} LogLevel;

void log_log(LogLevel level, char const* format, ...);

#endif // LOG_H_

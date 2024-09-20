#ifndef GL_ERRORS_H_
#define GL_ERRORS_H_

#define gl(name, ...)                        \
    do {                                     \
        gl_clear_errors();                   \
        gl##name(__VA_ARGS__);               \
        gl_check_errors(__FILE__, __LINE__); \
    } while (0);

#define gl_call(...)                         \
    do {                                     \
        gl_clear_errors();                   \
        __VA_ARGS__;                         \
        gl_check_errors(__FILE__, __LINE__); \
    } while (0);

void gl_clear_errors();
void gl_check_errors(const char* file, int line);

#endif // GL_ERRORS_H_
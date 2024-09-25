#include "gl_errors.h"

#include <GLES2/gl2.h>

#include "log.h"

void gl_clear_errors()
{
    while (glGetError() != GL_NO_ERROR)
        ;
}

void gl_check_errors(const char* file, int line)
{
    GLenum error;
    while ((error = glGetError()) != GL_NO_ERROR) {
        log_log(LOG_ERROR, "%s:%d: OpenGL error: error code 0x%X",
                file, line, error);
    }
}

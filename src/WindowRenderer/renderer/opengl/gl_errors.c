#include "gl_errors.h"

#include <stdio.h>

#include <GLES2/gl2.h>

void gl_clear_errors()
{
    while (glGetError() != GL_NO_ERROR);
}

void gl_check_errors(const char* file, int line)
{
    GLenum error;
    while ((error = glGetError()) != GL_NO_ERROR) {
        printf("%s:%d: ERROR: OpenGL error: error code 0x%X\n", 
               file, line, error);
    }
}

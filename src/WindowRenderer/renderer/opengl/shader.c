#include "shader.h"

#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <GLES2/gl2.h>

#include "gl_errors.h"
#include "log.h"

static GLuint compile_shader(GLenum type, const char* source)
{
    GLuint id;
    gl_call(id = glCreateShader(type));

    gl(ShaderSource, id, 1, &source, NULL);
    gl(CompileShader, id);

    int result;
    gl(GetShaderiv, id, GL_COMPILE_STATUS, &result);

    if (result == GL_FALSE) {
        int error_length;
        gl(GetShaderiv, id, GL_INFO_LOG_LENGTH, &error_length);

        char* error = (char*)alloca(error_length);
        gl(GetShaderInfoLog, id, error_length, &error_length, error);

        const char* shader_type = type == GL_VERTEX_SHADER ? "Vertex" : "Fragment";

        log_log(LOG_ERROR, "%s shader compilation: %s", shader_type, error);

        gl(DeleteShader, id);

        return 0;
    }

    return id;
}

static int shader_get_uniform_location(Shader* shader, const char* name)
{
    for (size_t i = 0; i < shader->uniform_count; ++i) {
        if (strcmp(shader->uniforms[i].name, name) == 0) {
            return shader->uniforms[i].location;
        }
    }

    int location;
    gl_call(location = glGetUniformLocation(shader->id, name));

    shader->uniforms[shader->uniform_count++] = (Uniform) {
        .name = name,
        .location = location,
    };

    return location;
}

Shader* shader_create(const char* vertex_source, const char* fragment_source)
{
    Shader* shader = malloc(sizeof(*shader));
    memset(shader, 0, sizeof(*shader));

    gl_call(shader->id = glCreateProgram());

    GLuint vert_shader = compile_shader(GL_VERTEX_SHADER, vertex_source);
    if (vert_shader == 0) {
        log_log(LOG_ERROR, "Failed to compile vertex shader");
        gl(DeleteProgram, shader->id);
        return NULL;
    }

    GLuint frag_shader = compile_shader(GL_FRAGMENT_SHADER, fragment_source);
    if (frag_shader == 0) {
        log_log(LOG_ERROR, "Failed to compile fragment shader");
        gl(DeleteProgram, shader->id);
        return NULL;
    }

    gl(AttachShader, shader->id, vert_shader);
    gl(AttachShader, shader->id, frag_shader);

    gl(BindAttribLocation, shader->id, 0, "a_position");
    gl(BindAttribLocation, shader->id, 1, "a_tex_coord");
    gl(BindAttribLocation, shader->id, 2, "a_color");

    gl(LinkProgram, shader->id);
    gl(ValidateProgram, shader->id);

    gl(DeleteShader, vert_shader);
    gl(DeleteShader, frag_shader);

    return shader;
}

void shader_destroy(Shader* shader)
{
    gl(DeleteProgram, shader->id);
    free(shader);
}

void shader_bind(Shader* shader)
{
    gl(UseProgram, shader->id);
}

void shader_unbind(Shader* shader)
{
    (void)shader;
    gl(UseProgram, 0);
}

void shader_set_uniform_1i(Shader* shader, const char* name, int x)
{
    gl(Uniform1i, shader_get_uniform_location(shader, name), x);
}

void shader_set_uniform_1f(Shader* shader, const char* name, float x)
{
    gl(Uniform1f, shader_get_uniform_location(shader, name), x);
}

void shader_set_uniform_2f(Shader* shader, const char* name, float x, float y)
{
    gl(Uniform2f, shader_get_uniform_location(shader, name),
       x, y);
}

void shader_set_uniform_3f(Shader* shader, const char* name,
                           float x, float y, float z)
{
    gl(Uniform3f, shader_get_uniform_location(shader, name),
       x, y, z);
}

void shader_set_uniform_4f(Shader* shader, const char* name,
                           float x, float y, float z, float w)
{
    gl(Uniform4f, shader_get_uniform_location(shader, name),
       x, y, z, w);
}

void shader_set_uniform_mat4x4f(Shader* shader, const char* name,
                                float m0, float m4, float m8, float m12,
                                float m1, float m5, float m9, float m13,
                                float m2, float m6, float m10, float m14,
                                float m3, float m7, float m11, float m15)
{
    float matrix[] = {
        m0, m4, m8, m12,
        m1, m5, m9, m13,
        m2, m6, m10, m14,
        m3, m7, m11, m15
    };
    gl(UniformMatrix4fv, shader_get_uniform_location(shader, name), 1, GL_FALSE, matrix);
}

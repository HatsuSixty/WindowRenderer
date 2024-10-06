#pragma once

#include <stdbool.h>
#include <stddef.h>

#include <GLES2/gl2.h>

typedef struct {
    int location;
    const char* name;
} Uniform;

#define MAX_UNIFORMS 1024

typedef struct {
    GLuint id;

    Uniform uniforms[MAX_UNIFORMS];
    size_t uniform_count;
} Shader;

// Returns NULL on error
Shader* shader_create(const char* vertex_source, const char* fragment_source);
void shader_destroy(Shader* shader);

void shader_bind(Shader* shader);
void shader_unbind(Shader* shader);

void shader_set_uniform_1i(Shader* shader, const char* name, int x);
void shader_set_uniform_1f(Shader* shader, const char* name, float x);
void shader_set_uniform_2f(Shader* shader, const char* name, float x, float y);
void shader_set_uniform_3f(Shader* shader, const char* name,
                           float x, float y, float z);
void shader_set_uniform_4f(Shader* shader, const char* name,
                           float x, float y, float z, float w);
void shader_set_uniform_mat4x4f(Shader* shader, const char* name,
                                float m0, float m4, float m8, float m12,
                                float m1, float m5, float m9, float m13,
                                float m2, float m6, float m10, float m14,
                                float m3, float m7, float m11, float m15);

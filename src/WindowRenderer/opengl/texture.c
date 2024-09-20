#include "texture.h"

#include <string.h>
#include <stdlib.h>

#include "gl_errors.h"

Texture* texture_create(unsigned char* pixels, int width, int height)
{
    Texture* texture = malloc(sizeof(*texture));
    memset(texture, 0, sizeof(*texture));

    texture->width = width;
    texture->height = height;

    gl(GenTextures, 1, &texture->id);

    gl(BindTexture, GL_TEXTURE_2D, texture->id);

    gl(TexParameteri, GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    gl(TexParameteri, GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    gl(TexParameteri, GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    gl(TexParameteri, GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    gl(TexImage2D, GL_TEXTURE_2D, 0, GL_RGBA, width, height,
                   0, GL_RGBA, GL_UNSIGNED_BYTE, pixels);

    gl(BindTexture, GL_TEXTURE_2D, 0);

    return texture;
}

void texture_destroy(Texture* texture)
{
    gl(DeleteBuffers, 1, &texture->id);
    free(texture);
}

void texture_bind(Texture* texture, int slot)
{
    gl(ActiveTexture, GL_TEXTURE0 + slot);
    gl(BindTexture, GL_TEXTURE_2D, texture->id);
}

void texture_unbind(Texture* texture)
{
    (void)texture;
    gl(BindTexture, GL_TEXTURE_2D, 0);
    gl(ActiveTexture, GL_TEXTURE0);
}
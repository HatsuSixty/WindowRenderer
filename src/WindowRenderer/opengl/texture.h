#ifndef TEXTURE_H_
#define TEXTURE_H_

#include <GLES2/gl2.h>

typedef struct {
    GLuint id;
    int width;
    int height;
} Texture;

Texture* texture_create(unsigned char* pixels, int width, int height);
void texture_bind(Texture* texture, int slot);
void texture_unbind(Texture* texture);
void texture_destroy(Texture* texture);

#endif // TEXTURE_H_
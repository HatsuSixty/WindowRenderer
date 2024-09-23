#ifndef TEXTURE_H_
#define TEXTURE_H_

#include <GLES2/gl2.h>
#include <GLES2/gl2ext.h>
#include <EGL/egl.h>
#include <EGL/eglext.h>

typedef struct {
    GLuint id;
    int width;
    int height;
} Texture;

Texture* texture_create(unsigned char* pixels, int width, int height);
Texture* texture_create_from_egl_imagekhr(EGLImageKHR egl_image, int width, int height,
                                          PFNGLEGLIMAGETARGETTEXTURE2DOESPROC glEGLImageTargetTexture2DOES);

void texture_bind(Texture* texture, int slot);
void texture_unbind(Texture* texture);
void texture_destroy(Texture* texture);

#endif // TEXTURE_H_

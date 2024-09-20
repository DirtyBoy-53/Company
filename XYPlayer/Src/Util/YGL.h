#ifndef _Y_GL_H_
#define _Y_GL_H_

#include "GL/glew.h"
#include "YFrame.h"
// GL PixelFormat extend
#define GL_I420             0x1910  // YYYYYYYYUUVV
#define GL_YV12             0x1911  // YYYYYYYYVVUU
#define GL_NV12             0x1912  // YYYYYYYYUVUV
#define GL_NV21             0x1913  // YYYYYYYYVUVU

typedef struct GLTexture_s {
    unsigned int id;  // for glGenTextures
    YFrame frame;
} GLTexture;
#endif
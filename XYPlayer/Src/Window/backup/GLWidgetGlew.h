#ifndef H_GL_WIDGET_GLEW_H
#define H_GL_WIDGET_GLEW_H

#include "YGL.h"
#include "YFrame.h"
#include "YGui.h"

#include <atomic>
#include <QOpenGLWidget>
#include "QMatrix4x4"
void bindTexture(GLTexture* tex, QImage* img);

class GLWidgetGlew : public QOpenGLWidget
{
public:
    GLWidgetGlew(QWidget* parent = NULL);

    // ratio = 0 means spread
    void setAspectRatio(double ratio);

    void drawFrame(YFramePtr pFrame);
    void drawTexture(HRect rc, GLTexture *tex);
    void drawRect(HRect rc, HColor clr, int line_width = 1, bool bFill = false);
    void drawText(QPoint lb, const char* text, int fontsize, QColor clr);

protected:
    virtual void initializeGL();
    virtual void resizeGL(int w, int h);
    virtual void paintGL();

    void setVertices(double ratio);
    void setVertices(QRect rc);

    static void loadYUVShader();
    void initVAO();
    void initYUV();

    void drawYUV(YFramePtr pFrame);

protected:
    static std::atomic_flag s_glew_init;
    static GLuint prog_yuv;
    static GLuint texUniformY;
    static GLuint texUniformU;
    static GLuint texUniformV;
    static GLuint matTransform;
    QMatrix4x4      m_transform;
    GLuint  tex_yuv[3];

    double  aspect_ratio;
    GLfloat vertices[8];
    GLfloat textures[8];

    // NOTE: QPainter used 3 VertexAttribArray
    enum VER_ATTR {
        VER_ATTR_VER = 3,
        VER_ATTR_TEX,
    };
    QSize m_size;
};

#endif // H_GL_WIDGET_GLEW_H

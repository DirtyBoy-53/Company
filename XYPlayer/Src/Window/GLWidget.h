#ifndef H_GL_WIDGET_GLEW_H
#define H_GL_WIDGET_GLEW_H


#include "YFrame.h"
#include <QOpenGLWidget>

class GLWidget : public QOpenGLWidget
{
public:
    // NOTE: QPainter used 3 VertexAttribArray
    enum VER_ATTR { VER_ATTR_VER = 3, VER_ATTR_TEX};

    GLWidget(QWidget* parent = NULL);
    ~GLWidget();
private:
    static void loadYUVShader();
    void initVAO();
    void initYUV();

    void drawYUV(YFrame *pFrame);

protected:
    virtual void initializeGL();
    virtual void resizeGL(int w, int h);
    virtual void paintGL();

    void drawFrame(YFrame *pFrame);

    void drawRect(QRect rc, QColor clr, int line_width = 1, bool bFill = false);
    void drawText(QPoint lb, const char* text, int fontsize, QColor clr);
    void drawImg(QImage &img);

private:
    struct Impl;
    std::unique_ptr<Impl> m_impl;
};

#endif // H_GL_WIDGET_GLEW_H

#ifndef GLWIDGET_H
#define GLWIDGET_H

#include <QOpenGLWidget>
#include <QOpenGLFunctions>
#include <memory>

class GLWidget : public QOpenGLWidget, protected QOpenGLFunctions
{
    Q_OBJECT
public:
    GLWidget(QWidget *parent = nullptr);
    ~GLWidget();

    void drawFrame(uint8_t *y, uint8_t *u, uint8_t *v, QSize size);

private:
    void InitializeTexture( int id, int width, int height);
    void initShader();
    void updateSize(QSize size);
    void drawYUV(uint8_t *y, uint8_t *u, uint8_t *v);

public slots:
    void setTransparent(bool transparent);

protected:
    void resizeGL(int w, int h) Q_DECL_OVERRIDE;
    void paintGL()              Q_DECL_OVERRIDE;
    void initializeGL()         Q_DECL_OVERRIDE;


private:
    struct Impl;
    std::unique_ptr<Impl> m_impl;

};

#endif // GLWIDGET_H

#include "GLWidget.h"
#include <QPaintEngine>
#include <QOpenGLShaderProgram>
#include <QOpenGLTexture>
#include <QCoreApplication>
#include <QOpenGLBuffer>
#include <QVector3D>
#include <QMatrix4x4>
#include <QVector>
#include <QOpenGLTexture>
#include <stdlib.h>
#include <QDebug>
struct GLWidget::Impl{
    QOpenGLShader*          m_vshaderA{nullptr};
    QOpenGLShader*          m_fshaderA{nullptr};
    QOpenGLShaderProgram*   m_programA{nullptr};
    QOpenGLBuffer           m_vboA;

    QOpenGLTexture*         m_texture{0};
    GLuint                  m_textureIds[3]{0};
    const char      m_indices[6] = { 0, 3, 2, 0, 2, 1 };
    GLfloat         m_verticesA[20] = {  1,  1, 0, 1, 0,
                                        -1,  1, 0, 0, 0,
                                        -1, -1, 0, 0, 1,
                                         1, -1, 0, 1, 1};

    bool            m_transparent{false};
    bool            m_isShaderInited{false};
    int             m_width{0};
    int             m_height{0};
};


void GLWidget::InitializeTexture( int id, int width, int height) {
    glBindTexture(GL_TEXTURE_2D, id);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_LUMINANCE, width, height, 0, GL_LUMINANCE,
                 GL_UNSIGNED_BYTE, NULL);
}

void GLWidget::initShader()
{
    m_impl->m_vshaderA = new QOpenGLShader(QOpenGLShader::Vertex);
    m_impl->m_vshaderA->compileSourceFile(":/vert");
    m_impl->m_fshaderA = new QOpenGLShader(QOpenGLShader::Fragment);
    m_impl->m_fshaderA->compileSourceFile(":/frag");
    m_impl->m_programA = new QOpenGLShaderProgram;
    m_impl->m_programA->addShader(m_impl->m_vshaderA);
    m_impl->m_programA->addShader(m_impl->m_fshaderA);
    m_impl->m_programA->link();

    int positionHandle = m_impl->m_programA->attributeLocation("aPosition");
    int textureHandle  = m_impl->m_programA->attributeLocation("aTextureCoord");

    auto size = 5 * sizeof(GLfloat);
    glVertexAttribPointer(positionHandle, 3, GL_FLOAT, false, size, m_impl->m_verticesA);
    glEnableVertexAttribArray(positionHandle);
    glVertexAttribPointer(textureHandle, 2, GL_FLOAT, false, size, &m_impl->m_verticesA[3]);
    glEnableVertexAttribArray(textureHandle);

    glUniform1i(m_impl->m_programA->uniformLocation("Ytex"), 0);
    glUniform1i(m_impl->m_programA->uniformLocation("Utex"), 1);
    glUniform1i(m_impl->m_programA->uniformLocation("Vtex"), 2);
}



GLWidget::GLWidget(QWidget *parent)
    : QOpenGLWidget(parent)
    , m_impl(std::make_unique<Impl>())
{
    memset(m_impl->m_textureIds,0,3);
}

GLWidget::~GLWidget()
{
    // And now release all OpenGL resources.
    makeCurrent();

    doneCurrent();
}



void GLWidget::initializeGL()
{
    initializeOpenGLFunctions();

    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_LINEAR);

    glEnable(GL_TEXTURE_2D);
    glClearColor(0.0f, 0.0f, 0.0f, 0.5f);

    glDepthFunc(GL_LEQUAL);
    glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST);

    initShader();
}

void GLWidget::drawYUV(uint8_t *y, uint8_t *u, uint8_t *v)
{
    auto w = m_impl->m_width, h = m_impl->m_height;
    if(!m_impl->m_isShaderInited) {
        m_impl->m_isShaderInited=true;
        glDeleteTextures(3, m_impl->m_textureIds);
        glGenTextures(3, m_impl->m_textureIds);
        glActiveTexture(GL_TEXTURE0);
        InitializeTexture( m_impl->m_textureIds[0], w, h);
        glActiveTexture(GL_TEXTURE1);
        InitializeTexture( m_impl->m_textureIds[1], w / 2, h / 2);
        glActiveTexture(GL_TEXTURE2);
        InitializeTexture( m_impl->m_textureIds[2], w / 2, h / 2);
    }
    m_impl->m_programA->bind();

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, m_impl->m_textureIds[0]);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_LUMINANCE, w, h, 0, GL_LUMINANCE, GL_UNSIGNED_BYTE, y);
    glUniform1i(m_impl->m_programA->uniformLocation("Ytex"), 0);

    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, m_impl->m_textureIds[1]);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_LUMINANCE, w/2, h/2, 0, GL_LUMINANCE, GL_UNSIGNED_BYTE, u);
    glUniform1i(m_impl->m_programA->uniformLocation("Utex"), 1);

    glActiveTexture(GL_TEXTURE2);
    glBindTexture(GL_TEXTURE_2D, m_impl->m_textureIds[2]);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_LUMINANCE, w/2, h/2, 0, GL_LUMINANCE, GL_UNSIGNED_BYTE, v);
    glUniform1i(m_impl->m_programA->uniformLocation("Vtex"), 2);

    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_BYTE, m_impl->m_indices);
    m_impl->m_programA->release();
}



void GLWidget::updateSize(QSize size)
{
    if(m_impl->m_width!=size.width() ||
        m_impl->m_height != size.height()) {
        m_impl->m_width= size.width();
        m_impl->m_height= size.height();
        m_impl->m_isShaderInited=false;
    }
}

void GLWidget::drawFrame(uint8_t *y, uint8_t *u, uint8_t *v, QSize size)
{
    qDebug() << "*******drawFrame******";
    updateSize(size);
    if(m_impl->m_width ==0 || m_impl->m_height ==0
        || !y || !u || !v) return;
    drawYUV(y,u,v);
}

void GLWidget::paintGL()
{
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

}



void GLWidget::setTransparent(bool transparent)
{
    setAttribute(Qt::WA_AlwaysStackOnTop, transparent);
    m_impl->m_transparent = transparent;
    window()->update();
}

void GLWidget::resizeGL(int w, int h)
{

}

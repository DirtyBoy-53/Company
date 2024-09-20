#include "YGL.h"
#include "GLWidget.h"
#include <QPainter>
#include <atomic>
#include <QMatrix4x4>
#include <QFile>
#include <QDebug>

#include "YDefine.h"

struct GLWidget::Impl{
    static std::atomic_flag s_glew_init;
    static GLuint           prog_yuv;
    static GLuint           texUniformY;
    static GLuint           texUniformU;
    static GLuint           texUniformV;
    static GLuint           matTransform;
    QMatrix4x4              m_transform;
    GLuint                  tex_yuv[3];

    double                  aspect_ratio;
    GLfloat                 vertices[8]{ -1.0f, -1.0f, 1.0f, -1.0f, -1.0f,  1.0f, 1.0f,  1.0f };
    GLfloat                 textures[8]{  0.0f,  1.0f, 1.0f,  1.0f,  0.0f,  0.0f, 1.0f,  0.0f };

    QSize                   m_size;

    static std::string loadSource(const std::string& filepath){
        QFile file(filepath.c_str());
        if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
            qDebug() << "Failed to open file:" << filepath.c_str();
            return "";
        }
        QTextStream in(&file);
        QString shaderCode = in.readAll();
        file.close();
        return shaderCode.toStdString();
    }

    static int glPixFmt(int type) {
        switch (type) {
        case PIX_FMT_BGR:  return GL_BGR;
        case PIX_FMT_RGB:  return GL_RGB;
        case PIX_FMT_BGRA: return GL_BGRA;
        case PIX_FMT_RGBA: return GL_RGBA;
        }
        return -1;
    }
};

std::atomic_flag GLWidget::Impl::s_glew_init = ATOMIC_FLAG_INIT;
GLuint GLWidget::Impl::prog_yuv;
GLuint GLWidget::Impl::texUniformY;
GLuint GLWidget::Impl::texUniformU;
GLuint GLWidget::Impl::texUniformV;
GLuint GLWidget::Impl::matTransform;

GLWidget::GLWidget(QWidget* parent)
    : QOpenGLWidget(parent)
    , m_impl(std::make_unique<Impl>())
{ }

GLWidget::~GLWidget(){}

void GLWidget::loadYUVShader() {
    GLuint vs = glCreateShader(GL_VERTEX_SHADER);
    GLuint fs = glCreateShader(GL_FRAGMENT_SHADER);

    std::string vsStr = Impl::loadSource(":/vert");
    const GLchar* pszVS = vsStr.c_str();
    GLint len = vsStr.size();
    glShaderSource(vs, 1, (const GLchar**)&pszVS, &len);

    std::string fsStr = Impl::loadSource(":/frag");
    const GLchar* pszFS = fsStr.c_str();
    len = fsStr.size();
    glShaderSource(fs, 1, (const GLchar**)&pszFS, &len);

    glCompileShader(vs);
    glCompileShader(fs);

    Impl::prog_yuv = glCreateProgram();

    glAttachShader(Impl::prog_yuv, vs);
    glAttachShader(Impl::prog_yuv, fs);

    glBindAttribLocation(Impl::prog_yuv, VER_ATTR_VER, "verIn");
    glBindAttribLocation(Impl::prog_yuv, VER_ATTR_TEX, "texIn");

    glLinkProgram(Impl::prog_yuv);
    glValidateProgram(Impl::prog_yuv);

    Impl::texUniformY = glGetUniformLocation(Impl::prog_yuv, "tex_y");
    Impl::texUniformU = glGetUniformLocation(Impl::prog_yuv, "tex_u");
    Impl::texUniformV = glGetUniformLocation(Impl::prog_yuv, "tex_v");
    Impl::matTransform = glGetUniformLocation(Impl::prog_yuv, "transform");
}

void GLWidget::initVAO() {
    glVertexAttribPointer(VER_ATTR_VER, 2, GL_FLOAT, GL_FALSE, 0, m_impl->vertices);
    glEnableVertexAttribArray(VER_ATTR_VER);

    glVertexAttribPointer(VER_ATTR_TEX, 2, GL_FLOAT, GL_FALSE, 0, m_impl->textures);
    glEnableVertexAttribArray(VER_ATTR_TEX);
}

void GLWidget::initYUV() {
    glGenTextures(3, m_impl->tex_yuv);
    for (int i = 0; i < 3; ++i) {
        glBindTexture(GL_TEXTURE_2D, m_impl->tex_yuv[i]);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER,   GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER,   GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S,       GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T,       GL_CLAMP_TO_EDGE);
    }
}

void GLWidget::initializeGL() {
    if (!m_impl->s_glew_init.test_and_set()) {
        if (glewInit() != 0) {
            m_impl->s_glew_init.clear();
            qFatal("glewInit failed");
            return;
        }
    }
    initVAO();

    loadYUVShader();
    initYUV();
}

void GLWidget::resizeGL(int w, int h) {
    glViewport(0,0,w,h);
    m_impl->m_transform.setToIdentity();
    float viewWidth=2.0f;
    float viewHeight=2.0f;
    int imgW = m_impl->m_size.width();
    int imgH = m_impl->m_size.height();
    if( imgW > 0 && imgH > 0) /// 在这里计算宽高比
    {
        float aspectRatio = 1.0*imgW/imgH;
        if(float(float(w)/h > aspectRatio)) {
            viewHeight = 2.0f;
            viewWidth = w*viewHeight / (aspectRatio * h);
        } else {
            viewWidth = 2.0f;
            viewHeight = h*viewWidth / (1/aspectRatio * w);
        }
    }

    m_impl->m_transform.ortho(-viewWidth/2,viewWidth/2,-viewHeight/2,viewHeight/2,-1.f,1.0f);
}

void GLWidget::paintGL() {
    glClearColor(0.0f, 0.0f, 0.0f, 0.5f);
    glClear(GL_COLOR_BUFFER_BIT);
}
#include <QDebug>
#include <QString>
void GLWidget::drawYUV(YFrame *pFrame) {
    assert(pFrame->type == PIX_FMT_IYUV || pFrame->type == PIX_FMT_YV12 ||
           pFrame->type == VIDEO_FORMAT_UYVY || pFrame->type == VIDEO_FORMAT_Y16);

    int w = pFrame->w;
    int h = pFrame->h;

    m_impl->m_size.setWidth(w);
    m_impl->m_size.setHeight(h);
    GLubyte* y = (GLubyte*)pFrame->getY();
    GLubyte* u = (GLubyte*)pFrame->getU();
    GLubyte* v = (GLubyte*)pFrame->getV();

    // qDebug("Y:0x%x  U:0x%x  V:0x%x", y, u, v);
    if (pFrame->type == PIX_FMT_YV12) {
        GLubyte* tmp = u;
        u = v;
        v = tmp;
    }

    glUseProgram(m_impl->prog_yuv);

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, m_impl->tex_yuv[0]);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RED, w, h, 0, GL_RED, GL_UNSIGNED_BYTE, y);
    glUniform1i(m_impl->texUniformY, 0);

    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, m_impl->tex_yuv[1]);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RED, w/2, h, 0, GL_RED, GL_UNSIGNED_BYTE, u);
    glUniform1i(m_impl->texUniformU, 1);

    glActiveTexture(GL_TEXTURE2);
    glBindTexture(GL_TEXTURE_2D, m_impl->tex_yuv[2]);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RED, w/2, h, 0, GL_RED, GL_UNSIGNED_BYTE, v);
    glUniform1i(m_impl->texUniformV, 2);

    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

    glUniformMatrix4fv(m_impl->matTransform, 1, GL_FALSE, m_impl->m_transform.constData());

    glUseProgram(0);
}

void GLWidget::drawFrame(YFrame *pFrame) {
    
    if (pFrame->type == PIX_FMT_IYUV || pFrame->type == PIX_FMT_YV12 ||
        pFrame->type == VIDEO_FORMAT_UYVY || pFrame->type == VIDEO_FORMAT_Y16) {
        drawYUV(pFrame);
    }
    else {
        glMatrixMode(GL_PROJECTION);
        glLoadIdentity();
        glRasterPos3f(-1.0f,1.0f,0);
        glPixelZoom(width()/(float)pFrame->w, -height()/(float)pFrame->h);
        glDrawPixels(pFrame->w, pFrame->h, Impl::glPixFmt(pFrame->type), GL_UNSIGNED_BYTE, pFrame->buf.base);
    }
}

void GLWidget::drawRect(QRect rc, QColor clr, int line_width, bool bFill) {
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glOrtho(0.0, width(), height(), 0.0, -1.0, 1.0);

    if (bFill) {
        glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    }
    else {
        glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
    }

    glLineWidth(line_width);

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    glColor4ub(clr.red(), clr.green(), clr.blue(), clr.alpha());
    glRecti(rc.left(), rc.top(), rc.right(), rc.bottom());
    glColor4ub(255,255,255,255);

    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    glDisable(GL_BLEND);
}


void GLWidget::drawText(QPoint lb, const char* text, int fontsize, QColor clr) {
    QPainter painter(this);
    QFont font = painter.font();
    font.setPointSize(fontsize);
    painter.setFont(font);
    painter.setPen(clr);
    painter.drawText(lb, text);
}

void GLWidget::drawImg(QImage &img)
{
    QPainter painter(this);
    painter.drawImage(QRect(x(),y(),width(),height()),img);
}

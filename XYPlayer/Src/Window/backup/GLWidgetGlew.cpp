#include "GLWidgetGlew.h"
#include <QPainter>

#include "YDefine.h"
static int glPixFmt(int type) {
    switch (type) {
    case PIX_FMT_BGR:  return GL_BGR;
    case PIX_FMT_RGB:  return GL_RGB;
    case PIX_FMT_BGRA: return GL_BGRA;
    case PIX_FMT_RGBA: return GL_RGBA;
    }
    return -1;
}

void bindTexture(GLTexture* tex, QImage* img) {
    if (img->format() != QImage::Format_ARGB32)
        return;

    glGenTextures(1, &tex->id);
    glBindTexture(GL_TEXTURE_2D, tex->id);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    tex->frame.w = img->width();
    tex->frame.h = img->height();
    tex->frame.type = GL_BGRA;
    tex->frame.depth = img->bitPlaneCount();
    // gluBuild2DMipmaps(GL_TEXTURE_2D, tex->frame.bpp/8, tex->frame.w, tex->frame.h, tex->frame.type, GL_UNSIGNED_BYTE, img->bits());
    glTexImage2D(GL_TEXTURE_2D, 0, tex->frame.depth/8, tex->frame.w, tex->frame.h, 0, tex->frame.type, GL_UNSIGNED_BYTE, img->bits());
}

std::atomic_flag GLWidgetGlew::s_glew_init = ATOMIC_FLAG_INIT;
GLuint GLWidgetGlew::prog_yuv;
GLuint GLWidgetGlew::texUniformY;
GLuint GLWidgetGlew::texUniformU;
GLuint GLWidgetGlew::texUniformV;
GLuint GLWidgetGlew::matTransform;

GLWidgetGlew::GLWidgetGlew(QWidget* parent)
    : QOpenGLWidget(parent)
{
    aspect_ratio = 0.0;
    setVertices(1.0);

    GLfloat tmp[] = {
        0.0f, 1.0f,
        1.0f, 1.0f,
        0.0f, 0.0f,
        1.0f, 0.0f,
    };

    // reverse
    /*
    GLfloat tmp[] = {
        0.0f, 0.0f,
        1.0f, 0.0f,
        0.0f, 1.0f,
        1.0f, 1.0f,
    };
    */
    memcpy(textures, tmp, sizeof(GLfloat)*8);
}

void GLWidgetGlew::setAspectRatio(double ratio) {
    aspect_ratio = ratio;
    if (abs(aspect_ratio) < 1e-6) {
        setVertices(1.0);
    }
    else {
        setVertices((double)height()/(double)width() * aspect_ratio);
    }
}

void GLWidgetGlew::setVertices(double ratio) {
    GLfloat w = 1.0, h = 1.0;
    if (ratio < 1.0) {
        w = ratio;
    }
    else {
        h = 1.0 / ratio;
    }

    GLfloat tmp[] = {
        -w, -h,
         w, -h,
        -w,  h,
         w,  h,
    };

    memcpy(vertices, tmp, sizeof(GLfloat)*8);
}

void GLWidgetGlew::setVertices(QRect rc) {
    int wnd_w = width();
    int wnd_h = height();
    if (wnd_w <= 0 || wnd_h <= 0) {
        return;
    }
    GLfloat left = (GLfloat)rc.left() * 2 / wnd_w - 1;
    GLfloat right = (GLfloat)(rc.right()+1) * 2 / wnd_w - 1;
    GLfloat top = 1 - (GLfloat)rc.top() * 2 / wnd_h;
    GLfloat bottom = 1 - (GLfloat)(rc.bottom()+1) * 2 / wnd_h;
    qDebug("l=%d r=%d t=%d b=%d", rc.left(), rc.right(), rc.top(), rc.bottom());
    qDebug("l=%f r=%f t=%f b=%f", left, right, top, bottom);
    GLfloat tmp[] = {
        left,  bottom,
        right, bottom,
        left,  top,
        right, top
    };

    memcpy(vertices, tmp, sizeof(GLfloat)*8);
}

void GLWidgetGlew::loadYUVShader() {
    GLuint vs = glCreateShader(GL_VERTEX_SHADER);
    GLuint fs = glCreateShader(GL_FRAGMENT_SHADER);
    char szVS[] = "				\
    attribute vec4 verIn;		\
    attribute vec2 texIn;		\
    varying vec2 texOut;		\
    uniform mat4 transform;     \
    void main(){				\
        gl_Position = transform * verIn;	\
        texOut = texIn;			\
    }							\
    ";
    const GLchar* pszVS = szVS;
    GLint len = strlen(szVS);
    glShaderSource(vs, 1, (const GLchar**)&pszVS, &len);
        // rgb = mat3( 1,        1,         1,			
        //             0, -0.39465,   2.03211,			
        //       1.13983, -0.58060,         0) * yuv;	
    char szFS[] = "				\
    varying vec2 texOut;		\
    uniform sampler2D tex_y;	\
    uniform sampler2D tex_u;	\
    uniform sampler2D tex_v;	\
                                \
    void main(){				\
        vec3 yuv;				\
        vec3 rgb;				\
        yuv.x = texture2D(tex_y, texOut).r;			\
        yuv.y = texture2D(tex_u, texOut).r - 0.5;	\
        yuv.z = texture2D(tex_v, texOut).r - 0.5;	\
        rgb = mat3( 1.0,        1.0,         1.0,			\
                    0, -0.344,   1.772,			\
              1.402, -0.714,         0.0) * yuv;	\
        gl_FragColor = vec4(rgb, 1.0);                \
    }												\
    ";
    const GLchar* pszFS = szFS;
    len = strlen(szFS);
    glShaderSource(fs, 1, (const GLchar**)&pszFS, &len);

    glCompileShader(vs);
    glCompileShader(fs);

//#ifdef _DEBUG
    GLint iRet = 0;
    glGetShaderiv(vs, GL_COMPILE_STATUS, &iRet);
    qDebug("vs::GL_COMPILE_STATUS=%d", iRet);
    glGetShaderiv(fs, GL_COMPILE_STATUS, &iRet);
    qDebug("fs::GL_COMPILE_STATUS=%d", iRet);
//#endif

    prog_yuv = glCreateProgram();

    glAttachShader(prog_yuv, vs);
    glAttachShader(prog_yuv, fs);

    glBindAttribLocation(prog_yuv, VER_ATTR_VER, "verIn");
    glBindAttribLocation(prog_yuv, VER_ATTR_TEX, "texIn");

    glLinkProgram(prog_yuv);

//#ifdef _DEBUG
    glGetProgramiv(prog_yuv, GL_LINK_STATUS, &iRet);
    qDebug("prog_yuv=%d GL_LINK_STATUS=%d", prog_yuv, iRet);
//#endif

    glValidateProgram(prog_yuv);

    texUniformY = glGetUniformLocation(prog_yuv, "tex_y");
    texUniformU = glGetUniformLocation(prog_yuv, "tex_u");
    texUniformV = glGetUniformLocation(prog_yuv, "tex_v");
    matTransform = glGetUniformLocation(prog_yuv, "transform");
    qDebug("loadYUVShader ok");
}

void GLWidgetGlew::initVAO() {
    glVertexAttribPointer(VER_ATTR_VER, 2, GL_FLOAT, GL_FALSE, 0, vertices);
    glEnableVertexAttribArray(VER_ATTR_VER);

    glVertexAttribPointer(VER_ATTR_TEX, 2, GL_FLOAT, GL_FALSE, 0, textures);
    glEnableVertexAttribArray(VER_ATTR_TEX);
}

void GLWidgetGlew::initYUV() {
    glGenTextures(3, tex_yuv);
    for (int i = 0; i < 3; ++i) {
        glBindTexture(GL_TEXTURE_2D, tex_yuv[i]);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    }
}

void GLWidgetGlew::initializeGL() {
    if (!s_glew_init.test_and_set()) {
        if (glewInit() != 0) {
            s_glew_init.clear();
            qFatal("glewInit failed");
            return;
        }
    }
    initVAO();

    loadYUVShader();
    initYUV();
}

void GLWidgetGlew::resizeGL(int w, int h) {
    glViewport(0,0,w,h);
    // setAspectRatio(aspect_ratio);
    m_transform.setToIdentity();
    float viewWidth=2.0f;
    float viewHeight=2.0f;
    int imgW = m_size.width();
    int imgH = m_size.height();
    if( imgW > 0 && imgH > 0) /// 在这里计算宽高比
    {
        float aspectRatio = 1.0*imgW/imgH;
        //aspectRatio = float(4) / 3; // 强制长宽比
        if(float(float(w)/h > aspectRatio))
        {
            viewHeight = 2.0f;
            viewWidth = w*viewHeight / (aspectRatio * h);
        }
        else
        {
            viewWidth = 2.0f;
            viewHeight = h*viewWidth / (1/aspectRatio * w);
        }
    }

    m_transform.ortho(-viewWidth/2,viewWidth/2,-viewHeight/2,viewHeight/2,-1.f,1.0f);
}

void GLWidgetGlew::paintGL() {
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);
}
#include <QDebug>
#include <QString>
void GLWidgetGlew::drawYUV(YFramePtr pFrame) {
    assert(pFrame->type == PIX_FMT_IYUV || pFrame->type == PIX_FMT_YV12 || VIDEO_FORMAT_UYVY);

    int w = pFrame->w;
    int h = pFrame->h;
    int y_size = w*h;
    m_size.setWidth(w);
    m_size.setHeight(h);
    GLubyte* y = (GLubyte*)pFrame->getY();
    GLubyte* u = (GLubyte*)pFrame->getU();
    GLubyte* v = (GLubyte*)pFrame->getV();

    // qDebug("Y:0x%x  U:0x%x  V:0x%x", y, u, v);
    if (pFrame->type == PIX_FMT_YV12) {
        GLubyte* tmp = u;
        u = v;
        v = tmp;
    }

    glUseProgram(prog_yuv);

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, tex_yuv[0]);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RED, w, h, 0, GL_RED, GL_UNSIGNED_BYTE, y);
    glUniform1i(texUniformY, 0);

    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, tex_yuv[1]);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RED, w/2, h, 0, GL_RED, GL_UNSIGNED_BYTE, u);
    glUniform1i(texUniformU, 1);

    glActiveTexture(GL_TEXTURE2);
    glBindTexture(GL_TEXTURE_2D, tex_yuv[2]);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RED, w/2, h, 0, GL_RED, GL_UNSIGNED_BYTE, v);
    glUniform1i(texUniformV, 2);

    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

    glUniformMatrix4fv(matTransform, 1, GL_FALSE, m_transform.constData());

    glUseProgram(0);
}

void GLWidgetGlew::drawFrame(YFramePtr pFrame) {
    
    if (pFrame->type == PIX_FMT_IYUV || pFrame->type == PIX_FMT_YV12 || VIDEO_FORMAT_UYVY) {
        drawYUV(pFrame);
    }
    else {
        glMatrixMode(GL_PROJECTION);
        glLoadIdentity();
        glRasterPos3f(-1.0f,1.0f,0);
        glPixelZoom(width()/(float)pFrame->w, -height()/(float)pFrame->h);
        glDrawPixels(pFrame->w, pFrame->h, glPixFmt(pFrame->type), GL_UNSIGNED_BYTE, pFrame->buf.base);
    }
}

void GLWidgetGlew::drawTexture(HRect rc, GLTexture *tex) {
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glOrtho(0.0, width(), height(), 0.0, -1.0, 1.0);

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, tex->id);

    glEnable(GL_TEXTURE_2D);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    glBegin(GL_QUADS);
    glTexCoord2d(0,0);glVertex2i(rc.left(), rc.top());
    glTexCoord2d(1,0);glVertex2i(rc.right(), rc.top());
    glTexCoord2d(1,1);glVertex2i(rc.right(), rc.bottom());
    glTexCoord2d(0,1);glVertex2i(rc.left(), rc.bottom());
    glEnd();

    glDisable(GL_TEXTURE_2D);
    glDisable(GL_BLEND);
}

void GLWidgetGlew::drawRect(HRect rc, HColor clr, int line_width, bool bFill) {
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

    glColor4ub(CLR_R(clr), CLR_G(clr), CLR_B(clr), CLR_A(clr));
    glRecti(rc.left(), rc.top(), rc.right(), rc.bottom());
    glColor4ub(255,255,255,255);

    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    glDisable(GL_BLEND);
}


void GLWidgetGlew::drawText(QPoint lb, const char* text, int fontsize, QColor clr) {
    QPainter painter(this);
    QFont font = painter.font();
    font.setPointSize(fontsize);
    painter.setFont(font);
    painter.setPen(clr);
    painter.drawText(lb, text);
}

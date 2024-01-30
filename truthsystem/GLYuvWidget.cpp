
#include "GLYuvWidget.h"
#include <QOpenGLShaderProgram>
#include <QOpenGLTexture>
#include <iostream>
#include <QSignalMapper>
#include <QEvent>
#include "ConStatus.hpp"
#include <QMessageBox>
#define VERTEXIN 0
#define TEXTUREIN 1


const char *vshadersrc ="attribute vec4 vertexIn; \
attribute vec2 textureIn; \
varying vec2 textureOut;  \
uniform mat4 transform; \
void main(void)           \
{                         \
    gl_Position = transform * vertexIn; \
    textureOut = textureIn; \
}";


const char *fshadersrc = "varying vec2 textureOut; \
uniform sampler2D tex_y; \
uniform sampler2D tex_u; \
uniform sampler2D tex_v; \
void main(void) \
{ \
    vec3 yuv; \
    vec3 rgb; \
    yuv.x = texture2D(tex_y, textureOut).r; \
    yuv.y = texture2D(tex_u, textureOut).r - 0.5; \
    yuv.z = texture2D(tex_v, textureOut).r - 0.5; \
    rgb = mat3( 1,       1,         1, \
                0,       -0.39465,  2.03211, \
                1.13983, -0.58060,  0) * yuv; \
    gl_FragColor = vec4(rgb, 1); \
}";

#include <QPushButton>

GLYuvWidget::GLYuvWidget(QWidget *parent, int videoId, bool needAssitant):
    QOpenGLWidget(parent), m_fullScreen(false), m_videoId(videoId)
{
    if (needAssitant) {
        m_assitant = new GLAssistant(nullptr, videoId);
        connect(m_assitant, &GLAssistant::signalSetYuv, this, &GLYuvWidget::slotSetYuv);
        connect(this, &GLYuvWidget::shotOne, m_assitant, &GLAssistant::slotShotOne);

        connect(GlobalSignals::getInstance(), &GlobalSignals::signal_one_frame_light, m_assitant, &GLAssistant::slotDataSetting);
    }
    connect(&m_timer_record,&QTimer::timeout,this,&GLYuvWidget::slot_timeout);
    m_timer_record.start(500);
    // QThread * pThread = new QThread;
    // this->moveToThread(pThread);
    // pThread->start();
}

GLYuvWidget::~GLYuvWidget()
{
    makeCurrent();
    vbo.destroy();
    textureY->destroy();
    textureU->destroy();
    textureV->destroy();
    delete program;
    doneCurrent();
    // qInfo("release yuv widget");
}

void GLYuvWidget::onShowYUV(uchar *Yptr, uchar *Uptr, uchar *Vptr, uint width, uint height, ScaleType type)
{
    //qDebug() << "GLYuvWidget::onShowYUV =》" << QThread::currentThreadId();
    // QMutexLocker locker(&m_showMutex);
    m_yPtr = Yptr;
    m_uPtr = Uptr;
    m_vPtr = Vptr;

    videoW = width;
    videoH = height;
    m_type = type;
    // qDebug() << "show YUV openGL>>>>>>>>>>>>>>>>>>>>>";
    update();
}
void GLYuvWidget::slotSetShowType(int slot, int type)
{

}

void GLYuvWidget::slot_timeout()
{
    
    static qint64 _count{0};
    static QTime _curTime;
    _count++;
    if(m_hasRecorded){
        if(_count%2 == 0){
            m_time_record = m_time_record.addSecs(1);
            m_label_record_time->setText(m_time_record.toString("hh:mm:ss"));
        }
    }
    if(m_hasPlayed){
        _curTime = QTime::currentTime();
        QTime _Time;
        if(CConStatus::instance()->get_connect_time(m_videoId,_Time)){
            int32_t elapsed = _Time.msecsTo(_curTime);
            //  qDebug() << QString("id:%1 data elapsed is %2").arg(m_videoId).arg(elapsed);
            if(elapsed < elapsed_max_num){
                // if(m_label_connect_status->isHidden())
                    m_label_connect_status->show();
                // else m_label_connect_status->hide();
            }else {
                m_label_connect_status->hide();
            }
        }
    }
}

void GLYuvWidget::slotSetData(int slot, unsigned char* data, int len)
{
}


void GLYuvWidget::slotSetYuv(int slot, uchar *Yptr,uchar *Uptr,uchar *Vptr,uint width,uint height, int type)
{
    if (slot != m_videoId) return;
    //qDebug() << "GLYuvWidget::onShowYUV =》" << QThread::currentThreadId();
    // QMutexLocker locker(&m_showMutex);
    m_yPtr = Yptr;
    m_uPtr = Uptr;
    m_vPtr = Vptr;

    videoW = width;
    videoH = height;
    m_type = (ScaleType)type;
    // qDebug() << "show YUV openGL>>>>>>>>>>>>>>>>>>>>>";
    update();
}
void GLYuvWidget::videoControl(QWidget* type)
{
    QPushButton* btn = (QPushButton*)type;
    if (btn->objectName() == "btnPlay") {
        // qDebug() << "play";
        if (!m_hasPlayed) {
            // btn->setText("停止");
            set_bt_Icon(m_btnPlay,":image/stop_play.png");
        } else {
            // btn->setText("播放");
            set_bt_Icon(m_btnPlay,":image/play.png");
        }
        m_hasPlayed = !m_hasPlayed;
    } else if (btn->objectName() == "btnRecord") {
        // qDebug() << "record";
        if(!m_hasPlayed){
             QMessageBox::information(NULL, "提示", "请开始播放后，再启用录制功能。");
             return;
        }
        if (!m_hasRecorded) {
            m_time_record = QTime::fromString("00:00:00","hh:mm:ss");
            m_label_record_time->setText(m_time_record.toString("hh:mm:ss"));
            // btn->setText("停录");
            m_videoRecord.start(m_videoId);
            set_bt_Icon(m_btnRecord,":image/stop_record.png");
        } else {
            
            // btn->setText("录制");
            m_videoRecord.stop();
            set_bt_Icon(m_btnRecord,":image/record.png");
        }
        m_hasRecorded = !m_hasRecorded;
    } else if (btn->objectName() == "btnShot") {
        if(!m_hasPlayed){
             QMessageBox::information(NULL, "提示", "请开始播放后，再启用截图功能。");
             return;
        }
        // qDebug() << "shot";
        emit shotOne(true);
    }
}

void GLYuvWidget::initializeGL()
{
    paintControl();
    m_isInitialized = false;

    initializeOpenGLFunctions();
    glEnable(GL_DEPTH_TEST);

    static const GLfloat vertices[]{
        //顶点坐标
        -1.0f,-1.0f,
        -1.0f,+1.0f,
        +1.0f,+1.0f,
        +1.0f,-1.0f,
        //纹理坐标
        0.0f,1.0f,
        0.0f,0.0f,
        1.0f,0.0f,
        1.0f,1.0f,
    };


    vbo.create();
    vbo.bind();
    vbo.allocate(vertices,sizeof(vertices));


    program = new QOpenGLShaderProgram(this);
     //Compile vertex shader
//    if (!program->addShaderFromSourceFile(QOpenGLShader::Vertex, ":/vs.glsl"))
//        close();

//    // Compile fragment shader
//    if (!program->addShaderFromSourceFile(QOpenGLShader::Fragment, ":/fs.glsl"))
//        close();

    program->addShaderFromSourceCode(QOpenGLShader::Vertex,vshadersrc);
    program->addShaderFromSourceCode(QOpenGLShader::Fragment,fshadersrc);

    //绑定输入的定点坐标和纹理坐标属性
    program->bindAttributeLocation("vertexIn",VERTEXIN);
    program->bindAttributeLocation("textureIn",TEXTUREIN);

    // Link shader pipeline
    if (!program->link())
    {
        qDebug()<<"program->link error";
        close();
    }


    // Bind shader pipeline for use
    if (!program->bind())
    {
        qDebug()<<"program->bind error";
        close();
    }

    //启用并且设置定点位置和纹理坐标
    program->enableAttributeArray(VERTEXIN);
    program->enableAttributeArray(TEXTUREIN);
    program->setAttributeBuffer(VERTEXIN,GL_FLOAT, 0, 2, 2*sizeof(GLfloat));
    program->setAttributeBuffer(TEXTUREIN,GL_FLOAT,8*sizeof(GLfloat), 2 , 2*sizeof(GLfloat));

    //定位shader中 uniform变量
    posUniformY = program->uniformLocation("tex_y");
    posUniformU = program->uniformLocation("tex_u");
    posUniformV = program->uniformLocation("tex_v");

    //创建纹理并且获取id
    textureY = new QOpenGLTexture(QOpenGLTexture::Target2D);
    textureU = new QOpenGLTexture(QOpenGLTexture::Target2D);
    textureV = new QOpenGLTexture(QOpenGLTexture::Target2D);
    textureY->create();
    textureU->create();
    textureV->create();
    m_idY = textureY->textureId();
    m_idU = textureU->textureId();
    m_idV = textureV->textureId();

    glClearColor(0.0,0.0,0.0,0.0);

    m_isInitialized = true;

}
#define judge(a) (a<0 ? 1 : a > 6000 ? 1:0)
#define isEmpty_(a) (a==nullptr ? 1 : 0)
void GLYuvWidget::paintGL()
{
    if (m_isInitialized && (m_hasPlayed || m_hasReplayed)) {
        //qDebug() << "GLYuvWidget::paintGL =》" << QThread::currentThreadId();

        QMatrix4x4 trans_;
        //2023-7-11 13:51:41 软件启动崩溃
        //qDebug() << ">>> Has Entered Paint Functions, videoW:" << videoW   <<"   videoH:"<<videoH << " -" << m_videoId;
        if(judge(videoW) || judge(videoH)){
            // qDebug() << "数据异常，图形超出限定长宽，" << "videoW:" << videoW   <<"   videoH:"<<videoH;
            return;
        }
        if(isEmpty_(m_yPtr) || isEmpty_(m_uPtr) || isEmpty_(m_vPtr)){
            // qDebug() << "图形数据为空：" << "m_yPtr:" << m_yPtr   <<"   m_uPtr:"<<m_uPtr<<"   m_vPtr:"<< m_vPtr;
            return;
        }


        float scale_im = (float)videoW/(float)videoH;
        float scale_form = (float)width()/(float)height();
        // qDebug() << "win_width: " << width()   <<"   wind_height:" << height();
        switch (m_type) {
        case fitXY:
            trans_.ortho(-1.0f,1.0f,-1.0f,1.0f,-1.0f,1.0f);
            break;
        case fitCenter:
            if(scale_im>scale_form)
                trans_.ortho(-1.0f,1.0f,-scale_im/scale_form,scale_im/scale_form,-1.0f,1.0f);
            else
                trans_.ortho(-scale_form/scale_im,scale_form/scale_im,-1.0f,1.0f,-1.0f,1.0f);
            break;
        case CenterCorp:
            if(scale_im>scale_form)
                trans_.ortho(-scale_form/scale_im,scale_form/scale_im,-1.0f,1.0f,-1.0f,1.0f);
            else
                trans_.ortho(-1.0f,1.0f,-scale_im/scale_form,scale_im/scale_form,-1.0f,1.0f);
            break;
        }
        program->setUniformValue("transform", trans_);
        //激活纹理单元GL_TEXTURE0,系统里面的
        glActiveTexture(GL_TEXTURE0);

        // qDebug() << "bind...";
        //绑定y分量纹理对象id到激活的纹理单元
        glBindTexture(GL_TEXTURE_2D, m_idY);


        ///------------------------
        /// glTexImage2D(
        /// GLenum target,
        /// GLint level,
        /// GLint internalformat,
        /// GLsizei width,
        /// GLsizei height,
        /// GLint border,
        /// GLenum format,
        /// GLenum type,
        /// const GLvoid* pixels)
        /* 函数很长，参数也不少，所以我们一个一个地讲解：
        第一个参数指定了纹理目标(Target)。设置为GL_TEXTURE_2D意味着会生成与当前绑定的纹理对象在同一个目标上的纹理
                （任何绑定到GL_TEXTURE_1D和GL_TEXTURE_3D的纹理不会受到影响）。
        第二个参数为纹理指定多级渐远纹理的级别，如果你希望单独手动设置每个多级渐远纹理的级别的话。这里我们填0，也就是基本级别。
        第三个参数告诉OpenGL我们希望把纹理储存为何种格式。
        第四个和第五个参数设置最终的纹理的宽度和高度。我们之前加载图像的时候储存了它们，所以我们使用对应的变量。
        第六个参数应该总是被设为0（历史遗留的问题）。
        第七个参数定义了源图的格式。
        第八个参数定义了源图的数据类型。
        最后一个参数是真正的图像数据。

        当调用glTexImage2D时，当前绑定的纹理对象就会被附加上纹理图像。然而，目前只有基本级别(Base-level)的纹理图像被加载了，
        如果要使用多级渐远纹理，我们必须手动设置所有不同的图像（不断递增第二个参数）。
        或者，直接在生成纹理之后调用glGenerateMipmap。这会为当前绑定的纹理自动生成所有需要的多级渐远纹理。
        */

        // qDebug() << "draw Y...";
        //使用内存中的数据创建真正的y分量纹理数据
        glTexImage2D(GL_TEXTURE_2D,0,GL_RED,videoW,videoH,0,GL_RED,GL_UNSIGNED_BYTE,m_yPtr);//m_yPtr 大小是(videoW*videoH)
        //https://blog.csdn.net/xipiaoyouzi/article/details/53584798 纹理参数解析
        glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

        // qDebug() << "draw U...";
        //激活纹理单元GL_TEXTURE1
        glActiveTexture(GL_TEXTURE1);
        //绑定u分量纹理对象id到激活的纹理单元
        glBindTexture(GL_TEXTURE_2D, m_idU);
        //使用内存中的数据创建真正的u分量纹理数据
        uint _videoH = m_assitant->m_videoFormat[m_videoId] == EnumFormat::Enum_Yuv444 ? videoH : videoH >> 1;
        glTexImage2D(GL_TEXTURE_2D,0,GL_RED,videoW>>1 , _videoH,0,GL_RED,GL_UNSIGNED_BYTE,m_uPtr);//m_uPtr 大小是(videoW*videoH / 2)
        glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

        // qDebug() << "draw V...";
        //激活纹理单元GL_TEXTURE2
        glActiveTexture(GL_TEXTURE2);
        //绑定v分量纹理对象id到激活的纹理单元
        glBindTexture(GL_TEXTURE_2D, m_idV);
        //使用内存中的数据创建真正的v分量纹理数据
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RED, videoW>>1 , _videoH, 0, GL_RED, GL_UNSIGNED_BYTE, m_vPtr);//m_vPtr 大小是(videoW*videoH / 2)
        glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

        // qDebug() << "draw new Y...";
        //指定y纹理要使用新值
        glUniform1i(posUniformY, 0);
        // qDebug() << "draw new U...";
        //指定u纹理要使用新值
        glUniform1i(posUniformU, 1);
        // qDebug() << "draw new V...";
        //指定v纹理要使用新值
        glUniform1i(posUniformV, 2);
        // qDebug() << "draw ...";
        //使用顶点数组方式绘制图形
        glDrawArrays(GL_TRIANGLE_FAN, 0, 4);
        // qDebug() << "draw over...";
    }
}

void GLYuvWidget::resizeGL(int w, int h)
{
    // qDebug() << "in resize GL, w: " << w << ", h: " << h;

    const float aspectRatio = ((float)w) / h;
    float xSpan = 1; // Feel free to change this to any xSpan you need.
    float ySpan = 1; // Feel free to change this to any ySpan you need.

    if (aspectRatio > 1){
        // Width > Height, so scale xSpan accordinly.
        xSpan *= aspectRatio;
    }
    else{
        // Height >= Width, so scale ySpan accordingly.
        ySpan = xSpan / aspectRatio;
    }

    glOrtho(-1*xSpan, xSpan, -1*ySpan, ySpan, -1, 1);

    // Use the entire window for rendering.
    glViewport(0, 0, w, h);

    QRect rect = this->geometry();
    QRect rectPlay;
    rectPlay.setX((rect.width() - 130) / 2);
    rectPlay.setY(rect.height() - 30);
    rectPlay.setWidth(30);
    rectPlay.setHeight(30);
    m_btnPlay->setGeometry(rectPlay);

    rectPlay.setX(rectPlay.x() + rectPlay.width() + 10);
    rectPlay.setY(rect.height() - 30);
    rectPlay.setWidth(30);
    rectPlay.setHeight(30);
    m_btnRecord->setGeometry(rectPlay);
    
    rectPlay.setX(rectPlay.x() + rectPlay.width() + 10);
    rectPlay.setY(rect.height() - 30);
    rectPlay.setWidth(30);
    rectPlay.setHeight(30);
    m_btnShot->setGeometry(rectPlay);

    QRect _rect = this->geometry();
    rectPlay.setX(50);
    rectPlay.setY(15);
    m_label_record_time->setGeometry(rectPlay);

    rectPlay.setX(10);
    rectPlay.setY(13);
    m_label_connect_status->setGeometry(rectPlay);

}

void GLYuvWidget::keyPressEvent(QKeyEvent *event)
{
}

void GLYuvWidget::mouseDoubleClickEvent(QMouseEvent *event)
{
    qDebug() << "mouse double clicked";
    // QMutexLocker locker(&m_showMutex);
    if (!m_fullScreen) {
        m_lastWindowFlags = this->windowFlags();
        m_lastGeometry = this->geometry();
        //this->setWindowFlags(Qt::Window);
        QWidget* prt = (QWidget*)this->parent();
        this->setGeometry(prt->geometry());
        raise();
        //setWindowFlags(m_lastWindowFlags | Qt::WindowStaysOnTopHint);
        //this->showFullScreen();
    } else {
        this->setWindowFlags(Qt::SubWindow);
        this->showNormal();
        this->setGeometry(m_lastGeometry);
    }
    m_fullScreen = !m_fullScreen;
}
void GLYuvWidget::set_bt_Icon(QPushButton *bt, QString path)
{
    bt->setIcon(QIcon(path));
    bt->setIconSize(QSize(20,20));
    bt->setShortcutEnabled(true);
    bt->setStyleSheet("border: 0px solid #dcdfe6;\nbackground-color: transparent;");
}
void GLYuvWidget::paintControl()
{
    // 画按钮
    m_btnPlay = new QPushButton(this);
    m_btnPlay->setObjectName("btnPlay");
    // m_btnPlay->setText("播放");
    QSignalMapper* mapper = new QSignalMapper;
    connect(m_btnPlay, SIGNAL(clicked()), mapper, SLOT(map()));
    mapper->setMapping(m_btnPlay, m_btnPlay);
    set_bt_Icon(m_btnPlay,":image/play.png");

    m_btnRecord = new QPushButton(this);
    m_btnRecord->setObjectName("btnRecord");
    // m_btnRecord->setText("录制");
    connect(m_btnRecord, SIGNAL(clicked()), mapper, SLOT(map()));
    mapper->setMapping(m_btnRecord, m_btnRecord);
    set_bt_Icon(m_btnRecord,":image/record.png");

    m_btnShot = new QPushButton(this);
    m_btnShot->setObjectName("btnShot");
    // m_btnShot->setText("截图");
    connect(m_btnShot, SIGNAL(clicked()), mapper, SLOT(map()));
    mapper->setMapping(m_btnShot, m_btnShot);
    set_bt_Icon(m_btnShot,":image/screenshot.png");

    connect(mapper, SIGNAL(mapped(QWidget*)), this, SLOT(videoControl(QWidget*)));
    videoControl(m_btnPlay);    //default play
    QRect rect = this->geometry();
    QRect rectPlay;
    rectPlay.setX((rect.width() - 130) / 2);
    rectPlay.setY(rect.height() - 30);
    rectPlay.setWidth(30);
    rectPlay.setHeight(30);
    m_btnPlay->setGeometry(rectPlay);


    rectPlay.setX(rectPlay.x() + rectPlay.width() + 10);
    rectPlay.setY(rect.height() - 30);
    rectPlay.setWidth(30);
    rectPlay.setHeight(30);
    m_btnRecord->setGeometry(rectPlay);

    
    rectPlay.setX(rectPlay.x() + rectPlay.width() + 10);
    rectPlay.setY(rect.height() - 30);
    rectPlay.setWidth(30);
    rectPlay.setHeight(30);
    m_btnShot->setGeometry(rectPlay);



    m_label_record_time = new QLabel(this);
    rectPlay.setX(50);
    rectPlay.setY(15);
    m_label_record_time->setGeometry(rectPlay);
    m_label_record_time->setMinimumSize(90,30);
    m_label_record_time->setMaximumSize(90,30);
    m_label_record_time->setStyleSheet("color:#FFFFFF;\nfont-size:20px;");

    m_label_connect_status = new QLabel(this);
    rectPlay.setX(10);
    rectPlay.setY(13);
    m_label_connect_status->setGeometry(rectPlay);
    m_label_connect_status->setMinimumSize(30,30);
    m_label_connect_status->setMaximumSize(30,30);
    m_label_connect_status->setStyleSheet("color:#FFFFFF;\nfont-size:20px;");
    m_img.load(":image/connect_status.png");
    QPixmap pix = QPixmap::fromImage(m_img);
    QPixmap fit = pix.scaled(m_label_connect_status->width(),m_label_connect_status->height(),Qt::IgnoreAspectRatio, Qt::SmoothTransformation);
    m_label_connect_status->setPixmap(fit);
    m_label_connect_status->hide();
}
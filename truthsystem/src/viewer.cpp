#include "viewer.h"
#if 1
#include <QMouseEvent>
#include "mathtypes.h"
#include <QGLViewer/manipulatedCameraFrame.h>
#include <QPushButton>
#include <GL/freeglut.h>

Viewer::Viewer(QWidget* parent, int slot, bool needAssitant) :
    QGLViewer(parent)
    ,_mPointsPtr(std::make_shared< pcl::PointCloud<PointXYZIT> >())
{
    qRegisterMetaType<StructPointData*>("StructPointData*");

}

void Viewer::init()
{
    // Used to display semi-transparent relection rectangle
    glBlendFunc(GL_ONE, GL_ONE);
    restoreStateFromFile();

    glDisable(GL_LIGHTING);

    setSceneRadius(100.0);          
    setSceneCenter(Vec(0, 0, 0));  

    camera()->showEntireScene();
    camera()->setZNearCoefficient(0.00001);
    camera()->setZClippingCoefficient(1000.0);
    camera()->setType(Camera::ORTHOGRAPHIC);//default projective mode is ortho

    set_camera_view(ENUM_VIEW::Enum_top);

    float colorrgb[] = { 33, 0, 33 };
    glClearColor(colorrgb[0] / 255, colorrgb[1] / 255, colorrgb[2] / 255, 0.0);

    glPointSize(2.0);
    setGridIsDrawn(false);//
    //setAxisIsDrawn(true);//can`t set axis size
    
    setFPSIsDisplayed(true);
    startAnimation();



}

void Viewer::updateViewMaster(PointView view)
{
    _mPointsPtr = view.pointsPtr;
    updateViewTime(view.frameTime);
}

void Viewer::set_camera_view(ENUM_VIEW view)
{
    setSceneCenter(Vec(0, 0, 0));
    double r = 0.0;
    switch (view) {
    case ENUM_VIEW::Enum_front:
        camera()->setUpVector(Vec(0, 0, 1));
        r = sqrt(pow(camera()->position().x, 2) + pow(camera()->position().y, 2) + pow(camera()->position().z, 2));
        camera()->setPosition(Vec(-r, 0, 0));
        camera()->lookAt(Vec(0, 0, 0));
        break;
    case ENUM_VIEW::Enum_right:
        camera()->setUpVector(Vec(0, 0, 1));
        r = sqrt(pow(camera()->position().x, 2) + pow(camera()->position().y, 2) + pow(camera()->position().z, 2));
        camera()->setPosition(Vec(0, -r, 0));
        camera()->lookAt(Vec(0, 0, 0));
        break;
    case ENUM_VIEW::Enum_top:
        camera()->setUpVector(Vec(1, 0, 0));
        r = sqrt(pow(camera()->position().x, 2) + pow(camera()->position().y, 2) + pow(camera()->position().z, 2));
        camera()->setPosition(Vec(0, 0, r));
        camera()->lookAt(Vec(0, 0, 0));
        break;
    default:break;
    }
}
void Viewer::drawTips(){
    if(m_timestamp != ""){
        QVector2D pos(width()-200, height()-20);
        set_display_text(pos, m_timestamp);
    }else{
        QVector2D pos(width()-200, height()-20);
        QString str_time("yyyy_MM_dd_hh_mm_ss_zzz");
        set_display_text(pos, str_time);
    }
    if(m_isShowRecord_flag){
        QVector2D pos(10, 40);
        glColor3f(248/255.0, 255.0, 82/255.0);
        QString str_time("实时保存中。。");
        set_display_text(pos, str_time);
    }else{
        QVector2D pos(10, 40);
        glColor3f(248/255.0, 255.0, 82/255.0);
        QString str_time("未保存");
        set_display_text(pos, str_time);
    }


    if (_find) {
        QVector2D pos(10, 60);
        glColor3f(248/255.0, 255.0, 82/255.0);
        QString str_time = QString("x:%1  y:%2  z:%3").arg(_findP.x).arg(_findP.y).arg(_findP.z);
        set_display_text(pos, str_time);
    }
}
void Viewer::draw()
{
    glDisable(GL_LIGHTING);
    
    drawAxis(10.0);
    //draw Grid  Circular
    drawGridAndCircular();

    glBegin(GL_POINTS);

    //glColor3f(1.0, 1.0, 1.0);//white color
    glPointSize(10.0f);
    float r = 1.0f, g = 1.0f, b = 1.0f;
    if(!_mPointsPtr.get()) return;
    size_t pointNum = _mPointsPtr->size();
    for (int i = 0; i < pointNum; i++)
    {
        convertRgbByIntensity(_mPointsPtr->at(i).intensity, r, g, b);
        glColor3f(r, g, b);
        glVertex3f(_mPointsPtr->at(i).x, _mPointsPtr->at(i).y, _mPointsPtr->at(i).z);
    }

    glEnd();
    
    drawTips();

    if(m_isShowCoordinate)
        drawCoordinate();
}

void Viewer::updateViewTime(QString time)
{
    m_timestamp = time;
}

void Viewer::drawGridAndCircular()
{
    float colorrgb[] = { 51, 51, 51 };
    glColor3f(colorrgb[0] / 255.0f, colorrgb[1] / 255.0f, colorrgb[2] / 255.0f);


    int nbSubdivisions = 2 * _nGrid;
    float size = _gridR * _nGrid;
    if (m_isShowGrid_flag) {
        glBegin(GL_LINES);
        for (int i = 0; i <= nbSubdivisions; ++i) {
            const float pos = size * (2.0 * i / nbSubdivisions - 1.0);
            glVertex2d(pos, -size);
            glVertex2d(pos, +size);
            glVertex2d(-size, pos);
            glVertex2d(size, pos);
        }
        glEnd();
    }

    int n = 180; 
    glLineWidth(1.5);
    if (m_isShowPolar_flag) {
        for (int num = 0; num < _nGrid; num++) {
            n = 100 + 50 * num;
            glBegin(GL_LINE_STRIP);

            for (int i = 0; i < n + 1; i++)
            {
                glVertex2f(_gridR * (num + 1) * cos(2 * PI * i / n), _gridR * (num + 1) * sin(2 * PI * i / n));
                //glVertex3f(R*(num+1)*cos(2 * PI*i / n), gridR*(num+1)*sin(2 * PI*i / n),0.0);
            }
            glEnd();
        }
        glColor3f(1.0f, 1.0f, 1.0f);
        QFont font;
        font.setBold(true);
        font.setPixelSize(20);
        for (auto i = 0; i < _nGrid; i++) {
            renderText(0.0, (i + 1) * _gridR, 0.5, QString("%1m").arg((i + 1) * 10), font);//left
            renderText(0.0, -(i + 1) * _gridR, 0.5, QString("%1m").arg((i + 1) * 10), font);//right
            renderText((i + 1) * _gridR, 0.0, 0.5, QString("%1m").arg((i + 1) * 10), font);//up
            renderText(-(i + 1) * _gridR, 0.0, 0.5, QString("%1m").arg((i + 1) * 10), font);//down
        }
    }
    





}

void Viewer::convertRgbByIntensity(float intensity, float& r, float& g, float& b)
{
    float f_max = 255;
    float f_min = 0;
    float range = f_max - f_min + 1;
    r = g = b = 0.0;

    float rtemp = (intensity - f_min) / range;
    int step = int(range / 4);
    int idx = int(rtemp * 4);
    float h = (idx + 1) * step + f_min;
    float m = idx * step + f_min;
    float local_r = (intensity - m) / (h - m);

    if (intensity < f_min) {
    }
    if (intensity > f_max) {
        r = g = b = 255.0;
    }
    if (idx == 0) {
        r = 0;
        g = int(local_r * 255);
        b = 255;
    }
    if (idx == 1) {
        r = 0;
        g = 255;
        b = int((1 - local_r) * 255);
    }
    if (idx == 2) {
        r = int(local_r * 255);
        g = 255;
        b = 0;
    }
    if (idx == 3) {
        r = 255;
        g = int((1 - local_r) * 255);
        b = 0;
    }

    r = r / 255.0;
    g = g / 255.0;
    b = b / 255.0;
    return;
}


void Viewer::resizeGL(int w, int h)
{
    QGLViewer::resizeGL(w, h);
}

void Viewer::mousePressEvent(QMouseEvent* e)
{
    QGLViewer::mousePressEvent(e); 
}

void Viewer::mouseMoveEvent(QMouseEvent* e)
{
    QGLViewer::mouseMoveEvent(e);
}

void Viewer::mouseReleaseEvent(QMouseEvent* e)
{
    //QGLViewer::mouseReleaseEvent(e);
}

void Viewer::postSelection(const QPoint& point)
{
    Vec orig, dir;
    camera()->convertClickToLine(point, orig, dir);

    GLfloat pDis = INF;
    FPoint3 oriP = FPoint3(orig.x, orig.y, orig.z);
    FPoint3 vector = FPoint3(dir.x, dir.y, dir.z);

    if(!_mPointsPtr.get()) return;
    for (int i = 0;i < _mPointsPtr->size();i++)
    {
        const PointXYZIT *p = &_mPointsPtr.get()->at(i);
        FPoint3 tmpS = FPoint3(p->x, p->y, p->z);
        FPoint3 fs = tmpS - oriP;

        GLfloat disTemp = vector.Cross(fs).Length() / vector.Length();
        if (disTemp < pDis)
        {
            pDis = disTemp;
            if (pDis < 0.3)
            {
                _findP = st_point(p->x, p->y, p->z);
                _find = true;
            }
        }
    }
}


void Viewer::drawCoordinate()
{
        int viewport[4];
        int scissor[4];

        // The viewport and the scissor are changed to fit the lower left
        // corner. Original values are saved.
        glGetIntegerv(GL_VIEWPORT, viewport);
        glGetIntegerv(GL_SCISSOR_BOX, scissor);

        // Axis viewport size, in pixels
        const int size = 80;
        glViewport(20, 20, size, size);
        glScissor(0, 0, size, size);

        // The Z-buffer is cleared to make the axis appear over the
        // original image.
        glClear(GL_DEPTH_BUFFER_BIT);

        // Tune for best line rendering
    //    glDisable(GL_LIGHTING);
        glLineWidth(2.0);

        glMatrixMode(GL_PROJECTION);
        glPushMatrix();
        glLoadIdentity();
        glOrtho(-1, 1, -1, 1, -1, 1);

        glMatrixMode(GL_MODELVIEW);
        glPushMatrix();
        glLoadIdentity();
        glMultMatrixd(camera()->orientation().inverse().matrix());

        glBegin(GL_LINES);
        glColor3f(1.0, 0.0, 0.0);//红
        glVertex3f(0.0, 0.0, 0.0);
        glVertex3f(1.0, 0.0, 0.0);
        
        glColor3f(1.0, 1.0, 0.0);//黄
        glVertex3f(0.0, 0.0, 0.0);
        glVertex3f(0.0, 1.0, 0.0);
        
        glColor3f(0.0, 1.0, 0.0);//绿
        glVertex3f(0.0, 0.0, 0.0);
        glVertex3f(0.0, 0.0, 1.0);
        glEnd();

        //绘制文字
        //glColor3f(1.0, 0.0, 0.0);//红
        //renderText(1.0, 0.0, 0.0, "X");
        //glColor3f(1.0, 1.0, 0.0);//黄
        //renderText(0.0, 1.0, 0.0, "Y");
        //glColor3f(0.0, 1.0, 0.0);//绿
        //renderText(0.0, 0.0, 1.0, "Z");

        glMatrixMode(GL_PROJECTION);
        glPopMatrix();

        glMatrixMode(GL_MODELVIEW);
        glPopMatrix();

        //  glEnable(GL_LIGHTING);

        // The viewport and the scissor are restored.
        glScissor(scissor[0], scissor[1], scissor[2], scissor[3]);
        glViewport(viewport[0], viewport[1], viewport[2], viewport[3]);
}

void Viewer::set_projection_mode(Camera::Type model){
        camera()->setType(model);
}
void Viewer::set_is_show_grid(bool state) {
    m_isShowGrid_flag = state;
}
void Viewer::set_is_show_polar(bool state) {
    m_isShowPolar_flag = state;
}
void Viewer::set_is_show_coordinate(bool state) {
    m_isShowCoordinate = state;
}

void Viewer::set_is_show_recordTips(bool state)
{
    m_isShowRecord_flag = state;
}

void Viewer::set_display_text(QVector2D &pos,QString &content) {

    glColor4f(foregroundColor().redF(), foregroundColor().greenF(),
        foregroundColor().blueF(), foregroundColor().alphaF());
    glDisable(GL_LIGHTING);
    
    drawText(pos.x(), pos.y(), content);
    glEnable(GL_LIGHTING);
}

#endif
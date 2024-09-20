#include "GLWindow.h"
#include <QDebug>
GLWindow::GLWindow(QWidget *parent)
    : GLWidget(parent)
{

}

void GLWindow::paintGL()
{
    static bool firstInto{false};
    GLWidget::paintGL();
    // qDebug() << "*************paintGL************";
    if(!frame.isNull()){

        // qDebug() << "*************draw frame paintGL************";
        try {
            drawFrame(&frame);
            drawFPS();
            drawRecord();
            drawTime();
            drawWatermask();
        } catch (...) {
            qDebug() << "绘制异常";
        }
        if(!firstInto){
            firstInto = true;
            GLWidget::resizeGL(width(),height());
        }

    }else{
        qDebug() << "*************Can`t draw frame at paintGL************";
    }
}

void GLWindow::drawTime()
{
    QPoint pt(width()-130,50);
    QString str = time.toString("hh:mm:ss");
    drawText(pt, str.toStdString().c_str(), 12, QColor(Qt::white));
}

void GLWindow::drawFPS()
{
    QPoint pt(width()-30,20);
    QString fpsStr = QString("%1 fps").arg(last_frame->fps);
    drawText(pt, fpsStr.toStdString().c_str(), 8, QColor(Qt::red));
}

void GLWindow::drawRecord()
{
    if(!draw_record) return;
    QPoint pt(width()-60,50);
    QString str = QString("REC");
    drawText(pt, str.toStdString().c_str(), 15, QColor(Qt::red));
}

void GLWindow::drawResolution()
{

}

void GLWindow::drawWatermask()
{
    for(auto i = 0;i < watermarkList.count();++i){
        CustomWatermarkInfo info = watermarkList.at(i);
        drawText(QPoint(info.x,info.y), info.value.toStdString().c_str(), info.fontSize, info.color);
    }
}


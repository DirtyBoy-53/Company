#ifndef GLWINDOW_H
#define GLWINDOW_H

#include "GLWidget.h"
#include "VideoWindow.h"
class GLWindow : public VideoWindow, GLWidget
{
public:
    GLWindow(QWidget* parent = nullptr);

    virtual void setGeometry(const QRect& rc) {
        GLWidget::setGeometry(rc);
    }

    virtual void update() {
        GLWidget::update();
    }

protected:
    virtual void paintGL();
    void drawTime();
    void drawFPS();
    void drawRecord();
    void drawResolution();
    void drawWatermask();
};

#endif // GLWINDOW_H

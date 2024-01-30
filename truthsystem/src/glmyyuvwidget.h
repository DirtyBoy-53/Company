#ifndef GLMyYuvWidget_H
#define GLMyYuvWidget_H


#include <QOpenGLWidget>
#include <QTimer>
#include "qttoopencv.h"

class GLMyYuvWidget : public QOpenGLWidget
{
    Q_OBJECT
public:
    GLMyYuvWidget(QWidget *parent =0);
    ~GLMyYuvWidget();

    void setWidgetIndex(const int index);
    void zoomSwitch(const bool flag);

public slots:
    void onShowImg(QImage img, int w, int h);

signals:
    void signalMousePress(const int index, QPoint point);
    void signalKeyDown(const int index, const int key);
    void signalMouseDoublePress(const int index);
    void signalWheelEvent(const int index, const int direction);

protected:
    void mousePressEvent(QMouseEvent *event);
//    void keyPressEvent(QKeyEvent *event);
    void mouseDoubleClickEvent(QMouseEvent *event);
    void wheelEvent(QWheelEvent *event);
    void paintEvent(QPaintEvent *e);


private:
    void startWheelTimer();
    void wheelTimeout();

private:
    int m_index = 0;

    bool m_initFlag = false;
    QTimer* m_wheelTimer = nullptr;
    int m_wheelDirection = 0;

    QPixmap m_curPixmap;
    float m_x,m_y;
    QtToOpencv m_qConvert;
    bool m_zoomFlag = true;
};


#endif // GLMyYuvWidget_H

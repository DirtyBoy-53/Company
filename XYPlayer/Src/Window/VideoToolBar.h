#ifndef VIDEOTOOLBAR_H
#define VIDEOTOOLBAR_H

#include <QFrame>
QT_FORWARD_DECLARE_CLASS(QLabel)
QT_FORWARD_DECLARE_CLASS(QPushButton)
QT_FORWARD_DECLARE_CLASS(QSlider)

class VideoToolBar : public QFrame
{
    Q_OBJECT
public:
     explicit VideoToolBar(QWidget *parent = nullptr);


signals:
    void sigStart();
    void sigPause();
    void sigStop();

protected:
    void initUI();
    void initConnect();

public:
    QPushButton*        m_btnStart;
    QPushButton*        m_btnPause;
    QPushButton*        m_btnRecord;
    QPushButton*        m_btnScreenShot;

    QSlider*            sldProgress;
    QLabel*             lblDuration;
    QPushButton*        m_btnFunWind;
};

#endif // VIDEOTOOLBAR_H

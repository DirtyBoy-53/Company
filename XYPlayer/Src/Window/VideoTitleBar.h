#ifndef VIDEOTITLEBAR_H
#define VIDEOTITLEBAR_H

#include <QFrame>
QT_FORWARD_DECLARE_CLASS(QLabel)
QT_FORWARD_DECLARE_CLASS(QPushButton)
class VideoTitleBar : public QFrame
{
    Q_OBJECT
public:
    explicit VideoTitleBar(QWidget *parent=nullptr);

protected:
    void initUI();
    void initConnect();

public:
    QLabel*         m_labTitle{nullptr};
    QPushButton*    m_btnClose{nullptr};
};

#endif // VIDEOTITLEBAR_H

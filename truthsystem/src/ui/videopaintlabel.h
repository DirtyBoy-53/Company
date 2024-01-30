#ifndef VIDEOPAINTLABEL_H
#define VIDEOPAINTLABEL_H

#include <QWidget>
#include <QImage>

class VideoPaintLabel : public QWidget
{
    Q_OBJECT
public:
    explicit VideoPaintLabel(QWidget *parent = nullptr);
    void paintEvent(QPaintEvent *event);
    void setImage(unsigned char* src, int width, int height);
    void setImage(QImage img);

private:
    QImage m_crtImage;
signals:

public slots:
};

#endif // VIDEOPAINTLABEL_H

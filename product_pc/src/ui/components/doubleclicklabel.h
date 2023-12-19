#ifndef DOUBLECLICKLABEL_H
#define DOUBLECLICKLABEL_H

#include <QLabel>
#include <QObject>
#include <QDialog>

class DoubleClickLabel : public QLabel
{
    Q_OBJECT
public:
    DoubleClickLabel();
    void mouseDoubleClickEvent(QMouseEvent *event);
    void enterEvent(QEvent *event);
    void leaveEvent(QEvent *event);

signals:
    void sigDoubleClicked();

private:
    QDialog * m_imageShow;
};

#endif // DOUBLECLICKLABEL_H

#ifndef CENTRALWIDGET_H
#define CENTRALWIDGET_H

#include <QWidget>
QT_FORWARD_DECLARE_CLASS(MulWindow)
QT_FORWARD_DECLARE_CLASS(LSideWindow)
class CentralWidget : public QWidget
{
    Q_OBJECT
public:
    explicit CentralWidget(QWidget *parent = nullptr);
    ~CentralWidget();
signals:
protected:
    void initUI();
    void initConnect();
public slots:

public:
    MulWindow*      m_mulWind{nullptr};
    LSideWindow*    m_lsideWind{nullptr};
    QWidget*        m_rsideWind{nullptr};
};

#endif // CENTRALWIDGET_H

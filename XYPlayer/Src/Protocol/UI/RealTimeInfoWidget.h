#ifndef REAL_TIME_INFO_WIDGET_H
#define REAL_TIME_INFO_WIDGET_H

#include <QWidget>
#include "Media.h"

QT_FORWARD_DECLARE_CLASS(YTableWidget)
QT_FORWARD_DECLARE_CLASS(QTextEdit)

class RealTimeInfoWidget : public QWidget
{
    Q_OBJECT
public:
    explicit RealTimeInfoWidget(QWidget *parent = nullptr);
    ~RealTimeInfoWidget();
    void initUI();
    void initConnect();

    void setMedia(Media &media);
public slots:
    void setParamLine(QByteArray param);

private:
    YTableWidget*       tableWidget;
    QTextEdit*          textEdit;
    Media               mMedia;
    
};

#endif // REAL_TIME_INFO_WIDGET_H

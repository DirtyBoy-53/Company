#ifndef COMMWIDGET_H
#define COMMWIDGET_H

#include <QWidget>
#include "Media.h"

QT_FORWARD_DECLARE_CLASS(QTabWidget)
QT_FORWARD_DECLARE_CLASS(ISPParamWidget)
QT_FORWARD_DECLARE_CLASS(FunSwitchWidget)
QT_FORWARD_DECLARE_CLASS(RealTimeInfoWidget)
QT_FORWARD_DECLARE_CLASS(CustomWidget)
QT_FORWARD_DECLARE_CLASS(DevInfoWidget)
QT_FORWARD_DECLARE_CLASS(ToolsWidget)


class CommWidget : public QWidget
{
    Q_OBJECT
public:
    explicit CommWidget(int id,QWidget *parent = nullptr);
    ~CommWidget();
    void setMedia(Media &media);
    void initUI();
    void initConnect();

    ToolsWidget*                toolsWidget{nullptr};
    RealTimeInfoWidget*         realTimeInfoWidget{nullptr};
signals:

public slots:

private:
    QTabWidget*                 tabWidgt{nullptr};
    ISPParamWidget*             ispParamWidget{nullptr};
    FunSwitchWidget*            funSwitchWidget{nullptr};
    
    CustomWidget*               customWidget{nullptr};
    DevInfoWidget*              devInfoWidget{nullptr};


    Media                       mMedia;
    int                         mId;

    // QWidget interface
protected:
    virtual void resizeEvent(QResizeEvent *event) override;
    virtual void moveEvent(QMoveEvent *event) override;
};

#endif // I2CCOMMWIDGET_H

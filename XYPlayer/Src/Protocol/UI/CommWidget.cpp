#include "CommWidget.h"
#include <QTabWidget>
#include <QHBoxLayout>
#include <QDebug>

#include "confile.h"
#include "ISPParamWidget.h"
#include "FunSwitchWidget.h"
#include "RealTimeInfoWidget.h"
#include "CustomWidget.h"
#include "DevInfoWidget.h"
#include "ToolsWidget.h"

CommWidget::CommWidget(int id,QWidget *parent)
    : QWidget(parent)
    , tabWidgt(new QTabWidget)
    , mId(id)
{
    initUI();
    initConnect();
    int x{100},y{100},w{600},h{800};
    QString key = QString("Comm_Widget_rect%1").arg(mId);
    std::string str = g_config->get<std::string>(key.toStdString(),"ui","rect(100,100,600,800)");

    if(!str.empty()){
        sscanf(str.c_str(),"rect(%d,%d,%d,%d)",&x,&y,&w,&h);
    }
    setGeometry(QRect(x,y,w,h));

}

CommWidget::~CommWidget()
{
    g_config->save();
}

void CommWidget::setMedia(Media &media)
{
    mMedia = media;
    ispParamWidget->setMedia(mMedia);
    funSwitchWidget->setMedia(mMedia);
    realTimeInfoWidget->setMedia(mMedia);
    customWidget->setMedia(mMedia);
    devInfoWidget->setMedia(mMedia);
    toolsWidget->setMedia(mMedia);
}

void CommWidget::initUI()
{
    ispParamWidget  = new ISPParamWidget(this);
    funSwitchWidget = new FunSwitchWidget(this);
    realTimeInfoWidget = new RealTimeInfoWidget(this);
    customWidget = new CustomWidget(this);
    devInfoWidget = new DevInfoWidget(this);
    toolsWidget = new ToolsWidget(this);

    tabWidgt->addTab(funSwitchWidget, "功能开关");
    tabWidgt->addTab(ispParamWidget, "ISP参数调试");
    tabWidgt->addTab(realTimeInfoWidget, "实时信息");
    tabWidgt->addTab(customWidget, "参数调试");
    tabWidgt->addTab(devInfoWidget, "设备信息");
    tabWidgt->addTab(toolsWidget, "便捷工具");

    QHBoxLayout *layout = new QHBoxLayout(this);
    layout->addWidget(tabWidgt);
}

void CommWidget::initConnect()
{

}

void CommWidget::resizeEvent(QResizeEvent *event)
{
    QWidget::resizeEvent(event);
    QString value = QString("rect(%1,%2,%3,%4)").arg(x()).arg(y()).arg(width()).arg(height());
    QString key = QString("Comm_Widget_rect%1").arg(mId);
    g_config->set<std::string>(key.toStdString(),value.toStdString(),"ui");
}

void CommWidget::moveEvent(QMoveEvent *event)
{
    QWidget::moveEvent(event);
    QString value = QString("rect(%1,%2,%3,%4)").arg(x()).arg(y()).arg(width()).arg(height());
    QString key = QString("Comm_Widget_rect%1").arg(mId);
    g_config->set<std::string>(key.toStdString(),value.toStdString(),"ui");
}

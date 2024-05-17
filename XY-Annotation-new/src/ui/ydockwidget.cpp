#include "ydockwidget.h"
#include <QLabel>
#include <QPushButton>
#include <QHBoxLayout>
YDockWidget::YDockWidget(QObject *parent) : QObject(parent)
{
    initUI();
}

void YDockWidget::initUI()
{

    //Qt标题栏
//    m_annoDockWidget.setWindowTitle("标注列表");
//    m_labelDockWidget.setWindowTitle("标签列表");
//    m_fileDockWidget.setWindowTitle("文件列表");

    //自定义标题栏
    m_annoDockWidget.setTitleBarWidget(new YTitleWidget("标注列表"));
    m_labelDockWidget.setTitleBarWidget(new YTitleWidget("标签列表"));
    m_fileDockWidget.setTitleBarWidget(new YTitleWidget("文件列表"));

    m_annoDockWidget .setWidget(&m_annoListWidget);
    m_labelDockWidget.setWidget(&m_labelListWidget);
    m_fileDockWidget .setWidget(&m_fileListWidget);
}


CustomListWidget &YDockWidget::fileListWidget() { return m_fileListWidget; }

const CustomListWidget &YDockWidget::labelListWidget() { return m_labelListWidget; }

const CustomListWidget &YDockWidget::annoListWidget() { return m_annoListWidget; }

QDockWidget &YDockWidget::fileDockWidget() { return m_fileDockWidget; }

QDockWidget &YDockWidget::labelDockWidget() { return m_labelDockWidget; }

QDockWidget &YDockWidget::annoDockWidget() { return m_annoDockWidget; }





YTitleWidget::YTitleWidget(QString title, QWidget *parent)
{
    Q_UNUSED(parent)
    initUI(title);
}

void YTitleWidget::initUI(QString title)
{
    auto SendLabel= new QLabel(this);         //标签
    SendLabel->setText(title);

    auto Btn=new QPushButton(this);    //按钮
    Btn->setFixedSize(20,20);
//    Btn->setIcon();
    Btn->setIconSize(QSize(13, 13));


    auto SendTitle_layout=new QHBoxLayout(this);    //布局
    SendTitle_layout->addWidget(SendLabel);
    SendTitle_layout->addStretch();
    SendTitle_layout->addWidget(Btn);
    SendTitle_layout->setContentsMargins(0,0,0,0);
    SendTitle_layout->setSpacing(0);
    setLayout(SendTitle_layout);
}

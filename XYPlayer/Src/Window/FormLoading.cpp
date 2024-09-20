#include "FormLoading.h"
#include <QMouseEvent>
#include <QTimer>
#include <QCoreApplication>
#include <QDebug>
#include <QThread>
#include <QPainter>
#include <QLabel>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QDialogButtonBox>
#include <QtConcurrent/QtConcurrent>
#include <thread>
#include <iostream>


#include "AlgSdkManager.h"
#include "YFunction.h"
#include "appdef.h"
#include "YDefine.h"
#include "confile.h"

FormLoading::FormLoading(QWidget *parent)
    : QDialog(parent)
    , m_timer_open(new QTimer(this))
    // , m_timer_init(new QTimer(this))
    // , m_future_open(new QFuture<bool>)
    // , m_future_init(new QFuture<bool>)
{
    initUI();
    initConnect();

    m_timer_open->setSingleShot(true);
    // m_timer_init->setSingleShot(true);
    m_timer_open->start(10);
    int x{300},y{300},w{600},h{600};
    std::string str = g_config->get<std::string>("formLoading_rect","ui","rect(300,300,600,600)");

    if(!str.empty()){
        sscanf(str.c_str(),"rect(%d,%d,%d,%d)",&x,&y,&w,&h);
    }
    setGeometry(QRect(x,y,w,h));
}

FormLoading::~FormLoading()
{
    qInfo() << "to be destroyed";
    AlgSdkManager::instance()->close();
    // delete m_future_open;
    // delete m_future_init;
    g_config->save();
}

void FormLoading::initUI()
{
    setWindowFlags(Qt::Window | Qt::FramelessWindowHint | Qt::WindowStaysOnTopHint |
                   Qt::Tool|Qt::X11BypassWindowManagerHint);
    setFixedSize(600,450);
    this->setWindowOpacity(0.99);

    auto lbl_background = genLabel(QPixmap(":formload"), size(), this);

    QString version = QString("%1 Version:%2").arg(APP_NAME).arg(APP_VERSION);
    auto lbl_copyright = new QLabel(COMPANY_COPYRIGHT);
    auto lbl_version   = new QLabel(version);
    auto lbl_title     = new QLabel(APP_NAME);
    lbl_outMsg         = new QLabel("");

    std::string lbl_style = R"(font: 9pt 'Microsoft YaHei UI';
                            color: rgb(104, 104, 104))";
    lbl_copyright->setStyleSheet(QString::fromStdString(lbl_style));
    lbl_version->setStyleSheet(QString::fromStdString(lbl_style));
    lbl_title->setStyleSheet("font:40pt 'Microsoft YaHei UI'; color: rgb(62, 37, 168)");

    btn_exit  = new QPushButton("退出");
    btn_enter = new QPushButton("进入");

    setBtnStyle(btn_enter,"#6a7777", "#FFFFFF", "#8fa2a2", "#F0F0F0", "#2D3E50", "#b7d0d0");
    setBtnStyle(btn_exit, "#6a7777", "#FFFFFF", "#8fa2a2", "#F0F0F0", "#2D3E50", "#b7d0d0");

    slot_setVisible(false);
    connect(btn_enter, &QPushButton::clicked, this, &QDialog::accept);
    connect(btn_exit,  &QPushButton::clicked, this, &FormLoading::close);

    auto hBox1 = genHBoxLayout();
    hBox1->addSpacing(70);
    hBox1->addWidget(lbl_copyright);
    hBox1->addStretch();
    hBox1->addWidget(lbl_version);
    hBox1->addSpacing(30);

    auto vBox1 = genVBoxLayout();
    vBox1->addWidget(lbl_title);
    vBox1->addSpacing(60);
    vBox1->addWidget(lbl_outMsg);
    vBox1->addSpacing(60);

    auto hBox2 = genHBoxLayout();
    hBox2->addSpacing(250);
    hBox2->addLayout(vBox1);

    auto btn_hBox = genHBoxLayout();
    btn_hBox->addStretch();
    btn_hBox->addWidget(btn_enter);
    btn_hBox->addSpacing(20);
    btn_hBox->addWidget(btn_exit);
    btn_hBox->addSpacing(50);

    auto vBox2 = genVBoxLayout();
    vBox2->addStretch();
    vBox2->addLayout(hBox2);
    vBox2->addLayout(btn_hBox);
    vBox2->addSpacing(60);
    vBox2->addLayout(hBox1);
    vBox2->addSpacing(10);

    setLayout(vBox2);
}

void FormLoading::initConnect()
{
    connect(&m_futureWatcher_init, &QFutureWatcher<bool>::started, this, &FormLoading::slot_algSdkInitStart);
    connect(&m_futureWatcher_init, &QFutureWatcher<bool>::finished, this, &FormLoading::slot_algSdkInitFinished);

    connect(&m_futureWatcher_open, &QFutureWatcher<bool>::started, this, &FormLoading::slot_algSdkOpenStart);
    connect(&m_futureWatcher_open, &QFutureWatcher<bool>::finished, this, &FormLoading::slot_algSdkOpenFinished);

    connect(m_timer_open, &QTimer::timeout, this, &FormLoading::slot_OnULoaded_open);
    // connect(m_timer_init, &QTimer::timeout, this, &FormLoading::slot_OnULoaded_init);
}

void FormLoading::mousePressEvent(QMouseEvent *event)
{
    if(event->button() == Qt::LeftButton)
    {
        m_bDrag = true;

        mouseStartPoint = event->globalPos();

        windowTopLeftPoint = this->frameGeometry().topLeft();
    }
}

void FormLoading::mouseMoveEvent(QMouseEvent *event)
{
    if(m_bDrag)
    {
        QPoint distance = event->globalPos() - mouseStartPoint;

        this->move(windowTopLeftPoint + distance);
    }
}

void FormLoading::mouseReleaseEvent(QMouseEvent *event)
{
    if(event->button() == Qt::LeftButton)
    {
        m_bDrag = false;
    }
}

void FormLoading::showEvent(QShowEvent *)
{
    //    _timer->start(1);
}

bool FormLoading::eventFilter(QObject *target, QEvent *event)
{
    if(event->type() == QEvent::KeyPress)
    {
         QKeyEvent *k = static_cast<QKeyEvent *>(event);

         if(k->key() == Qt::Key_Return || k->key() == Qt::Key_Enter)
         {
             QDialog::accept();
             return true;
         }
    }
    return QWidget::eventFilter(target, event);
}



void FormLoading::slot_OnULoaded_open()
{
    slot_OnUpdateInfo("艾利光PCIE采集卡SDK初始化 已开始.....", MSG_NORMAL);
    AlgSdkManager::instance()->open();
    QThread::sleep(2);
    AlgSdkManager::instance()->init();
    slot_OnUpdateInfo("艾利光PCIE采集卡SDK初始化 成功", MSG_WARNING);
    slot_setVisible(true);
    // *m_future_open = QtConcurrent::run([=](){
    //     std::cout << "open thread id:" << getpid() << "\n";
    //     return AlgSdkManager::instance()->open();
    // });
    // m_futureWatcher_open.setFuture(*m_future_open);
    // m_timer_open->deleteLater();
    // m_timer_init->start(2000);
    // slot_setVisible(true);
}

void FormLoading::slot_OnULoaded_init()
{


    // std::cout << "init slot_OnULoaded_init id:" << getpid() << "\n";
    // *m_future_init = QtConcurrent::run([=](){
    //     std::cout << "init thread id:" << getpid() << "\n";
    //     return AlgSdkManager::instance()->init();
    // });
    // m_futureWatcher_init.setFuture(*m_future_init);
    // m_timer_init->deleteLater();

    slot_setVisible(true);
}


void FormLoading::slot_OnUpdateInfo(const QString &msg, int type)
{
    std::string format;
    if(type == MSG_NORMAL){
        format = R"(font: 10pt 'Microsoft YaHei UI';
                 color: #7d7d7d)";
    }else if(type == MSG_WARNING){
        format = R"(font: 12pt 'Microsoft YaHei UI';
                 color: #eb704d)";
    }else if(type == MSG_SUCCESS){
        format = R"(font: 12pt 'Microsoft YaHei UI';
                 color: #4debaf)";
    }
    lbl_outMsg->setStyleSheet(QString::fromStdString(format));
    lbl_outMsg->setText(msg);
    lbl_outMsg->repaint();
}

void FormLoading::slot_setVisible(bool state)
{
    btn_enter->setVisible(state);
    btn_exit->setVisible(state);
}

void FormLoading::slot_algSdkInitStart()
{
    slot_OnUpdateInfo("艾利光PCIE采集卡SDK初始化 已开始.....", MSG_NORMAL);
    QThread::msleep(10);
    slot_OnUpdateInfo("艾利光PCIE采集卡SDK初始化 成功", MSG_WARNING);
}

void FormLoading::slot_algSdkInitFinished()
{
    if(m_future_init->result()){
        slot_OnUpdateInfo("艾利光PCIE采集卡SDK初始化 失败", MSG_WARNING);
    }
    slot_setVisible(true);
}

void FormLoading::slot_algSdkOpenStart()
{
    slot_OnUpdateInfo("艾利光PCIE采集卡SDK连接 已开始.....", MSG_NORMAL);
}

void FormLoading::slot_algSdkOpenFinished()
{
    if(m_future_init->result()){
        slot_OnUpdateInfo("艾利光PCIE采集卡SDK连接 失败", MSG_WARNING);
    }
    slot_setVisible(true);
}

void FormLoading::resizeEvent(QResizeEvent *event)
{
    QDialog::resizeEvent(event);
    QString value = QString("rect(%1,%2,%3,%4)").arg(x()).arg(y()).arg(width()).arg(height());
    g_config->set<std::string>("formLoading_rect",value.toStdString(),"ui");
}

void FormLoading::moveEvent(QMoveEvent *event)
{
    QDialog::moveEvent(event);
    QString value = QString("rect(%1,%2,%3,%4)").arg(x()).arg(y()).arg(width()).arg(height());
    g_config->set<std::string>("formLoading_rect",value.toStdString(),"ui");
}



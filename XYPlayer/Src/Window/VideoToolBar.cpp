#include "VideoToolBar.h"
#include "styles.h"
#include "YFunction.h"

#include <QHBoxLayout>

VideoToolBar::VideoToolBar(QWidget *parent)
{
    initUI();
    initConnect();
}

void VideoToolBar::initUI()
{
    setFixedHeight(VIDEO_TOOLBAR_HEIGHT);

    QSize sz(VIDEO_TOOLBAR_ICON_WIDTH, VIDEO_TOOLBAR_ICON_HEIGHT);
    m_btnStart  = genPushButton(QPixmap(":start"), "开始");
    m_btnPause  = genPushButton(QPixmap(":pause"), "停止");
    m_btnRecord = genPushButton(QPixmap(":record"), "录制");
    m_btnScreenShot = genPushButton(QPixmap(":shot"), "截图");
    m_btnFunWind    = genPushButton(QPixmap(":fun"), "多功能");
    QHBoxLayout *hbox = genHBoxLayout();
    hbox->setSpacing(5);
    hbox->addWidget(m_btnStart,  0, Qt::AlignLeft);
    hbox->addWidget(m_btnPause,  0, Qt::AlignLeft);

    m_btnPause->hide();

    sldProgress = new QSlider;
    sldProgress->setOrientation(Qt::Horizontal);
    lblDuration = new QLabel("00:00:00");
    hbox->addSpacing(5);
    hbox->addWidget(sldProgress);
    sldProgress->hide();
    hbox->addWidget(lblDuration);
    lblDuration->hide();

    hbox->addWidget(m_btnRecord);
    hbox->addSpacing(5);
    hbox->addWidget(m_btnScreenShot);
    hbox->addSpacing(5);
    hbox->addWidget(m_btnFunWind);
    hbox->addSpacing(5);
    setLayout(hbox);
}

void VideoToolBar::initConnect()
{
    connectButtons(m_btnStart, m_btnPause);

    connect(m_btnStart, SIGNAL(clicked(bool)), this, SIGNAL(sigStart()));
    connect(m_btnPause, SIGNAL(clicked(bool)), this, SIGNAL(sigPause()));
}

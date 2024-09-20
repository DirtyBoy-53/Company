#include "VideoTitleBar.h"
#include "styles.h"
#include "YFunction.h"

#include <QHBoxLayout>
VideoTitleBar::VideoTitleBar(QWidget *parent)
    : m_labTitle(new QLabel)
{
    initUI();
    initConnect();
}

void VideoTitleBar::initUI()
{
    setFixedHeight(VIDEO_TITLEBAR_HEIGHT);

    m_btnClose = genPushButton(QPixmap(":close"), "close");

    QHBoxLayout *hbox = genHBoxLayout();
    hbox->addWidget(m_labTitle);
    hbox->addStretch();
    hbox->addWidget(m_btnClose);

    setLayout(hbox);
}

void VideoTitleBar::initConnect()
{

}

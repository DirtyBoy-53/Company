#include "DevInfoWidget.h"
#include <QLineEdit>
#include <QPushButton>
#include <QLabel>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QDebug>
#include <QGridLayout>


#include "I2CManager.h"
#include "IICAddr.h"
#include "YFunction.h"

DevInfoWidget::DevInfoWidget(QWidget *parent)
    : QWidget{parent}
{
    initUI();
    initConnect();
}

DevInfoWidget::~DevInfoWidget()
{

}

void DevInfoWidget::initUI()
{

    mPlatformType = new QLineEdit();
    mAsicVersion = new QLineEdit();
    mReolution = new QLineEdit();
    mProductName = new QLineEdit();


    auto vbox = new QVBoxLayout();

    auto gridLayout = new QGridLayout();
    gridLayout->addWidget(new QLabel("平台类型:"),  0, 0);
    gridLayout->addWidget(mPlatformType,    0, 1);
    gridLayout->addWidget(new QLabel("ASIC版本:"),  0, 2);
    gridLayout->addWidget(mAsicVersion,     0, 3);
    gridLayout->addWidget(new QLabel("分辨率:"), 1, 0);
    gridLayout->addWidget(mReolution,       1, 1);
    gridLayout->addWidget(new QLabel("产品名称:"),  1, 2);
    gridLayout->addWidget(mProductName,     1, 3);

    vbox->addLayout(gridLayout);
    vbox->addSpacing(5);

    mBtnRead = new QPushButton("读取");
    vbox->addWidget(mBtnRead);
    vbox->addStretch();
    setLayout(vbox);

}

void DevInfoWidget::initConnect()
{
    connect(mBtnRead, &QPushButton::clicked, this, &DevInfoWidget::on_btn_clicked);
}

void DevInfoWidget::setMedia(Media &media)
{
    mMedia = media;
}
#define HIGH8(data) ((data) >> 8)
#define LOW8(data) ((data) & 0xff)
void DevInfoWidget::on_btn_clicked()
{
    I2CManager i2cManager;
    uint16_t size, value;
    QByteArray data;
    bool ok{false};

    //平台信息
    size = 2;
    data = i2cManager.read(mMedia.channel, DEVICE_INFO_PLATFORM, size);
    value = byteArrayToUint16(data, ok);
    if(!ok){
        mPlatformType->setText("");           
    } else {
        mPlatformType->setText(value == 0 ? "Asic 1.0":"Asic 2.0");
    }
    //ASIC版本
    auto year = i2cManager.read(mMedia.channel, DEVICE_INFO_SOFTWARE_VERSION0, size);//年份
    auto date = i2cManager.read(mMedia.channel, DEVICE_INFO_SOFTWARE_VERSION1, size);//月日
    auto ver = i2cManager.read(mMedia.channel, DEVICE_INFO_SOFTWARE_VERSION2, size);//主+次版本
    auto rver = i2cManager.read(mMedia.channel, DEVICE_INFO_SOFTWARE_VERSION3, size);//修订+预留
    uint16_t yearValue = byteArrayToUint16(year, ok);
    uint16_t dateValue = byteArrayToUint16(date, ok);
    uint16_t verValue = byteArrayToUint16(ver, ok);
    uint16_t rverValue = byteArrayToUint16(rver, ok);
    QString version = QString::asprintf("V%d.%d.%d.%04d%02d%02d", HIGH8(verValue), LOW8(verValue), HIGH8(rverValue), //主 次 修
                                                            yearValue, HIGH8(dateValue), LOW8(dateValue));//年 月 日
    mAsicVersion->setText(version);

    //分辨率    
    auto col = i2cManager.read(mMedia.channel, DEVICE_INFO_VIDEO_COL, size);
    auto row = i2cManager.read(mMedia.channel, DEVICE_INFO_VIDEO_ROW, size);
    uint16_t colValue = byteArrayToUint16(col, ok);
    uint16_t rowValue = byteArrayToUint16(row, ok);
    mReolution->setText(QString::asprintf("%dx%d", colValue, rowValue));
    
    //产品名称
    auto typeH = i2cManager.read(mMedia.channel, DEVICE_INFO_APP_TYPE_H, size);
    auto typeL = i2cManager.read(mMedia.channel, DEVICE_INFO_APP_TYPE_L, size);
    uint16_t typeHValue = byteArrayToUint16(typeH, ok);
    uint16_t typeLValue = byteArrayToUint16(typeL, ok);
    QString str1(HIGH8(typeHValue));
    QString str2(LOW8(typeHValue));
    QString str3 = QString("%1").arg(HIGH8(typeLValue), 2, 10, QChar('0'));
    QString str4(LOW8(typeLValue));
    mProductName->setText(str1.toUpper() + str2.toUpper() + str3 + str4.toUpper());
}

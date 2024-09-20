#include "CustomWidget.h"
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QTextEdit>
#include <QButtonGroup>
#include <QCheckBox>
#include "I2CManager.h"
#include "YFunction.h"


I2CRegisterOptBar::I2CRegisterOptBar(QWidget *parent)
    : QFrame(parent)
{
    initUI();
    initConnect();
}

I2CRegisterOptBar::~I2CRegisterOptBar()
{

}

void I2CRegisterOptBar::setRegOpt(RegOpt opt)
{
    mRegOpt = opt;
    updateUI();
}

void I2CRegisterOptBar::initUI()
{
    setFixedHeight(300);
    mlabelTips = new QLabel("除读取长度为10进制外,其余数据皆为16进制");

    mDataLabel = new QLabel("读取长度");

    mDevAddrLabel = new QLabel("设备地址");
    mRegAddrLabel = new QLabel("寄存器地址");

    mDevAddr = new QLineEdit("67");
    mRegAddr = new QLineEdit("");

    
    mBtn = new QPushButton("读取");
    
    mData = new QLineEdit();


    mTextBox = new QTextEdit();
    auto hbox1 = new QHBoxLayout();
    hbox1->addWidget(mDevAddrLabel);
    hbox1->addWidget(mDevAddr);
    hbox1->addSpacing(20);
    hbox1->addWidget(mRegAddrLabel);
    hbox1->addWidget(mRegAddr);

    auto hbox2 = new QHBoxLayout();
    hbox2->addWidget(mDataLabel);
    hbox2->addWidget(mData);
    hbox2->addSpacing(20);
    hbox2->addWidget(mBtn);
    hbox2->addStretch();

    auto vbox = new QVBoxLayout();
    vbox->addWidget(mlabelTips);
    vbox->addLayout(hbox1);
    vbox->addSpacing(5);
    vbox->addLayout(hbox2);
    vbox->addSpacing(5);
    vbox->addWidget(mTextBox);
    setLayout(vbox);

}

void I2CRegisterOptBar::updateUI()
{
    if(mRegOpt == RegOpt::REG_OPT_READ)
    {
        mlabelTips->setText("除读取长度为10进制外,其余数据皆为16进制");
        mDataLabel->setText("读取长度");
        mBtn->setText("读取");
        mData->setText("");
        mDevAddr->setText("67");
    } else {
        mlabelTips->setText("所有数据皆为16进制");
        mDataLabel->setText("写入数据");
        mBtn->setText("写入");
        mData->setText("");
        mDevAddr->setText("66");
    }
}

void I2CRegisterOptBar::initConnect()
{

}

CustomWidget::CustomWidget(QWidget *parent)
    : QWidget(parent)
    , mRegOptBar(new I2CRegisterOptBar)
{
    initUI();
    initConnect();
}

CustomWidget::~CustomWidget()
{
}

void CustomWidget::initUI()
{
    checkBox_read      = new QCheckBox("读数据");
    checkBox_write      = new QCheckBox("写数据");

    auto btnGroup = new QButtonGroup();
    btnGroup->addButton(checkBox_read, 0);
    btnGroup->addButton(checkBox_write, 1);
    checkBox_read->setCheckState(Qt::Checked);
    btnGroup->setExclusive(true);

    auto layoutGroup = new QHBoxLayout;
    layoutGroup->addWidget(checkBox_read);
    layoutGroup->addWidget(checkBox_write);

    auto layout = new QVBoxLayout();
    layout->addLayout(layoutGroup);
    layout->addSpacing(5);
    layout->addWidget(mRegOptBar);
    layout->addStretch();
    setLayout(layout);
}

void CustomWidget::initConnect()
{
    connect(checkBox_read, &QCheckBox::stateChanged, this, &CustomWidget::on_checkBox_clicked);
    connect(checkBox_write, &QCheckBox::stateChanged, this, &CustomWidget::on_checkBox_clicked);

    connect(mRegOptBar->mBtn, &QPushButton::clicked, this, &CustomWidget::on_btn_clicked);
}

void CustomWidget::setMedia(Media &media)
{
    mMedia = media;
}

void CustomWidget::on_checkBox_clicked(bool check)
{
    Q_UNUSED(check)
    QCheckBox *checkBox = qobject_cast<QCheckBox*>(sender());
    QString str = checkBox->text();
    auto opt = str == "读数据" ? I2CRegisterOptBar::REG_OPT_READ :  I2CRegisterOptBar::REG_OPT_WRITE;
    mRegOptBar->setRegOpt(opt);
}

void CustomWidget::on_btn_clicked()
{
    QPushButton *btn = qobject_cast<QPushButton*>(sender());
    QString str = btn->text();
    I2CManager i2cManager;
    bool ok;
    uint16_t value,regAddr,devAddr;
    regAddr = mRegOptBar->mRegAddr->text().toUInt(&ok, 16);
    if(!ok) goto ERROR_END;

    devAddr  = mRegOptBar->mDevAddr->text().toUInt(&ok, 16);
    if(!ok) goto ERROR_END;

    if(str == "写入"){
        value = mRegOptBar->mData->text().toUInt(&ok, 16);
        if(!ok) goto ERROR_END;
        if(!i2cManager.send(mMedia.channel, regAddr, value, devAddr)){
            yprint("写入失败");
        }

    }else if(str == "读取"){
        value = mRegOptBar->mData->text().toUInt(&ok, 10);
        if(!ok) goto ERROR_END;

        QByteArray data = i2cManager.read(mMedia.channel, regAddr, value, devAddr);
        mRegOptBar->mTextBox->setText(byteArrayToHexStr(data));
        if(data.size() <= 0){
            yprint("读取失败");
        }
    }
    return;

ERROR_END:
        yprint("参数错误，请检查后重新设置");

}

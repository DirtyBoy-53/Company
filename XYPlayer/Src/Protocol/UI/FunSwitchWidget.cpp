#include "FunSwitchWidget.h"
#include <QCheckBox>
#include <QButtonGroup>
#include <QPushButton>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QDebug>
#include "YFunction.h"
#include "I2CManager.h"
#include "IICAddr.h"

FunSwitchWidget::FunSwitchWidget(QWidget *parent)
    : QWidget(parent)
{
    initUI();
    initConnect();
}

FunSwitchWidget::~FunSwitchWidget()
{}

void FunSwitchWidget::setMedia(Media &media)
{
    mMedia = media;
}

void FunSwitchWidget::initUI()
{
    auto layout = new QVBoxLayout(this);

    // 伪彩
    checkBox_white      = new QCheckBox("白热");
    checkBox_black      = new QCheckBox("黑热");
    checkBox_rainbow    = new QCheckBox("彩虹");
    checkBox_green      = new QCheckBox("绿热");
    checkBox_blue       = new QCheckBox("蓝热");

    auto btnGroup_Color = new QButtonGroup(this);
    btnGroup_Color->addButton(checkBox_white, 0);
    btnGroup_Color->addButton(checkBox_black, 1);
    btnGroup_Color->addButton(checkBox_rainbow, 2);
    btnGroup_Color->addButton(checkBox_green, 3);
    btnGroup_Color->addButton(checkBox_blue, 4);

    btnGroup_Color->setExclusive(true);

    auto layout_color = new QHBoxLayout;
    layout_color->addWidget(checkBox_white);
    layout_color->addWidget(checkBox_black);
    layout_color->addWidget(checkBox_rainbow);
    layout_color->addWidget(checkBox_green);
    layout_color->addWidget(checkBox_blue);

    // 快门开关
    btn_shutter_open = new QPushButton("快门开");
    btn_shutter_close = new QPushButton("快门关");

    QHBoxLayout *hbox = genHBoxLayout();
    hbox->setSpacing(5);
    hbox->addWidget(btn_shutter_open,   0, Qt::AlignLeft);
    hbox->addWidget(btn_shutter_close,  0, Qt::AlignLeft);
    btn_shutter_close->hide();

    // 算法开关
    hbox->setSpacing(5);
    btn_algorithm_open = new QPushButton("一键算法开");
    btn_algorithm_close = new QPushButton("一键算法关");
    hbox->addWidget(btn_algorithm_open,   0, Qt::AlignLeft);
    hbox->addWidget(btn_algorithm_close,  0, Qt::AlignLeft);
    btn_algorithm_close->hide();

    // 视频格式
    hbox->setSpacing(5);
    btn_video_format_y8 = new QPushButton("Y8");
    btn_video_format_y16 = new QPushButton("Y16");
    hbox->addWidget(btn_video_format_y8,   0, Qt::AlignLeft);
    hbox->addWidget(btn_video_format_y16,  0, Qt::AlignLeft);
    btn_video_format_y16->hide();

    //NUC
    btn_nuc_recoup = new QPushButton("NUC");
    hbox->addWidget(btn_nuc_recoup, 0, Qt::AlignLeft);

    // 快门补偿
    btn_shutter_recoup = new QPushButton("快门补偿");
    hbox->addWidget(btn_shutter_recoup, 0, Qt::AlignLeft);

    
    layout->addLayout(layout_color);
    layout->addSpacing(10);
    layout->addLayout(hbox);
    layout->addStretch();
}

void FunSwitchWidget::initConnect()
{
    // 伪彩
    connect(checkBox_white, &QCheckBox::stateChanged, this, &FunSwitchWidget::on_checkBox_clicked);
    connect(checkBox_black, &QCheckBox::stateChanged, this, &FunSwitchWidget::on_checkBox_clicked);
    connect(checkBox_rainbow , &QCheckBox::stateChanged, this, &FunSwitchWidget::on_checkBox_clicked);
    connect(checkBox_green, &QCheckBox::stateChanged, this, &FunSwitchWidget::on_checkBox_clicked);
    connect(checkBox_blue , &QCheckBox::stateChanged, this, &FunSwitchWidget::on_checkBox_clicked);

    // 快门开关
    connectButtons(btn_shutter_open, btn_shutter_close);
    connect(btn_shutter_open,  &QPushButton::clicked, this, &FunSwitchWidget::on_btn_clicked);
    connect(btn_shutter_close, &QPushButton::clicked, this, &FunSwitchWidget::on_btn_clicked);

    // 算法开关
    connectButtons(btn_algorithm_open, btn_algorithm_close);
    connect(btn_algorithm_open,  &QPushButton::clicked, this, &FunSwitchWidget::on_btn_clicked);
    connect(btn_algorithm_close, &QPushButton::clicked, this, &FunSwitchWidget::on_btn_clicked);

    // 视频格式
    connectButtons(btn_video_format_y8, btn_video_format_y16);
    connect(btn_video_format_y8,  &QPushButton::clicked, this, &FunSwitchWidget::on_btn_clicked);
    connect(btn_video_format_y16, &QPushButton::clicked, this, &FunSwitchWidget::on_btn_clicked);

    //NUC
    connect(btn_nuc_recoup,  &QPushButton::clicked, this, &FunSwitchWidget::on_btn_clicked);
    
    // 快门补偿
    connect(btn_shutter_recoup,  &QPushButton::clicked, this, &FunSwitchWidget::on_btn_clicked);
    
}

void FunSwitchWidget::on_checkBox_clicked(bool checked)
{
    Q_UNUSED(checked)
    QCheckBox *checkBox = qobject_cast<QCheckBox*>(sender());
    QString str = checkBox->text();
    I2CManager i2cManager;
    uint16_t value{0};
    int regAddr{IMAGE_SETTING_SET_COLOR};
    if(str == "白热") {
        value = 0;
    } else if(str == "黑热"){
        value = 1;
    } else if(str == "彩虹"){
        value = 12;
    } else if(str == "绿热"){
        value = 21;
    } else if(str == "蓝热"){
        value = 22;
    } else {  }

    if(!i2cManager.send(mMedia.channel, regAddr, value)){
        yprint("伪彩设置失败");
    }
}

void FunSwitchWidget::on_btn_clicked()
{
    QPushButton *btn = qobject_cast<QPushButton*>(sender());
    QString str = btn->text();
    I2CManager i2cManager;
    uint8_t value{0};
    int regAddr{SYSTEM_CONTROL_SHUTTER_SWITCH};
    if(str == "快门开") {
        regAddr = SYSTEM_CONTROL_SHUTTER_SWITCH;
        value = 0;
    } else if(str == "快门关"){
        regAddr = SYSTEM_CONTROL_SHUTTER_SWITCH;
        value = 1;
    } else if(str == "一键算法开"){
        regAddr = IMAGE_CONFIG_SWITCH_ALG;
        value = 1;
    } else if(str == "一键算法关"){
        regAddr = IMAGE_CONFIG_SWITCH_ALG;
        value = 0;
    } else if(str == "Y8"){
        regAddr = SYSTEM_CONTROL_VIDEO_FORMAT;
        value = 1;
    } else if(str == "Y16"){
        regAddr = SYSTEM_CONTROL_VIDEO_FORMAT;
        value = 3;
    } else if(str == "NUC"){
        regAddr = IMAGE_CORRECTION_NUC;
        value = 1;
    } else if(str == "快门补偿"){
        regAddr = IMAGE_CORRECTION_SHUTTER_COMP;
        value = 1;
    }      
    else { }

    if(!i2cManager.send(mMedia.channel, regAddr, value)){
        yprint("发送失败");
    }
}

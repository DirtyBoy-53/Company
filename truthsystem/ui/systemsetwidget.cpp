#include "systemsetwidget.h"
#include "ui_systemsetwidget.h"
#include "configinfo.h"
#include <QDebug>
#include <QDateTime>
#include <QFileDialog>
#include <QCheckBox>
#include <QThread>
#include <QMessageBox>
#include "log.h"
#ifdef __linux__
#include "memcache.h"
#endif
#include "initeractionbyhttp.h"
SystemSetWidget::SystemSetWidget(QWidget *parent)
    : QWidget(parent), ui(new Ui::SystemSetWidget)
{
    ui->setupUi(this);
    Init();
    slot_setPath_btnClicked();

    //进制该页面上的滚轮事件，防止误触：
    auto qComboBoxGroup= this->findChildren<QComboBox*>();
    for(auto each:qComboBoxGroup)
    {
    each->installEventFilter(this);
    }


}
bool SystemSetWidget::eventFilter(QObject *target, QEvent *event)
{
    //屏蔽鼠标滚轮事件
    if(event->type() == QEvent::Wheel&&(target->inherits("QComboBox")))
    {
        return true;
    }
    return false;
}
SystemSetWidget::~SystemSetWidget()
{
    delete ui;
}

void SystemSetWidget::Init()
{
    ConfigInfo::getInstance()->Init();
    TableInit();
    TreeWidgetInit();

    QString _saveDir = ConfigInfo::getInstance()->m_saveDir;
    if(_saveDir.isEmpty()){
        _saveDir = "/media/xy/raid0";
    }
    ui->lineEdit_savePath->setText(_saveDir);
    emit signal_sendSavePath(_saveDir);

    connect(ui->btnSaveCollect, &QPushButton::clicked, this, &SystemSetWidget::slot_SaveSenceParam_btClicked);
    // connect(ui->btn_savePath, &QPushButton::clicked, this, &SystemSetWidget::slot_setPath_btnClicked);
    ui->btn_savePath->hide();//取消设置按钮
    connect(ui->btn_browse, &QPushButton::clicked, this, &SystemSetWidget::slot_browsePath_btnClicked);
}

#pragma region 标签表格处理
void SystemSetWidget::TableAddData(QTableWidget *widget, QString name, QString content, QString brief, QComboBox *box)
{
    auto RowCont = ui->tableWidget_SenceInfo->rowCount();
    widget->insertRow(RowCont); // 增加一行

    // 设置单元格字体颜色、背景颜色和字体字符：
    QTableWidgetItem *item_name = new QTableWidgetItem(name);
    QTableWidgetItem *item_content = new QTableWidgetItem(content);
    QTableWidgetItem *item_brief = new QTableWidgetItem(brief);
    ////    if(RowCont%2 == 0){
    //        item_name->setBackgroundColor(QColor(220,255,255));
    //        item_content->setBackgroundColor(QColor(220,255,255));
    //        item_brief->setBackgroundColor(QColor(220,255,255));
    ////    }

    // 插入元素
    widget->setItem(RowCont, 0, item_name);
    widget->setItem(RowCont, 1, item_content);
    widget->setItem(RowCont, 2, item_brief);

    widget->item(RowCont, 0)->setTextAlignment(Qt::AlignHCenter | Qt::AlignVCenter);
    widget->item(RowCont, 1)->setTextAlignment(Qt::AlignHCenter | Qt::AlignVCenter);
    if (box)
    {
        widget->setCellWidget(RowCont, 1, box);
    }
}

int QStringListMatch(QStringList &list, QString &str)
{
    int _idx = 0;
    for (auto idx = 0; idx < list.size(); idx++)
    {
        if (str == list.at(idx))
        {
            _idx = idx;
            break;
        }
    }
    return _idx;
}

void SystemSetWidget::TableInit()
{
    // Table界面初始化
    ui->tableWidget_SenceInfo->setColumnCount(2); // 名称 内容 提示
    ui->tableWidget_SenceInfo->setHorizontalHeaderLabels(QStringList() << QString::fromUtf8("名称")
                                                                       << QString::fromUtf8("内容")
                                                                       << QString::fromUtf8("提示"));
    // 设置列宽
    ui->tableWidget_SenceInfo->setColumnWidth(0, 120);
    ui->tableWidget_SenceInfo->setColumnWidth(1, 150);

    // 整行选中的方式
    ui->tableWidget_SenceInfo->setSelectionBehavior(QAbstractItemView::SelectRows);

    // 去除选中虚线框
    ui->tableWidget_SenceInfo->setFocusPolicy(Qt::NoFocus);

    // 填入配置文件中的信息
    CSenceInfo _senceInfo;
    ConfigInfo::getInstance()->GetSenceInfo(_senceInfo);

    TableAddData(ui->tableWidget_SenceInfo, ("日期"), QDateTime::currentDateTime().toString("yyyy_MM_dd"), QString(""));
    QComboBox *_weaterbox = new QComboBox();
    _weaterbox->addItems(ConfigInfo::getInstance()->m_listWeather);
    _weaterbox->setCurrentIndex(QStringListMatch(ConfigInfo::getInstance()->m_listWeather, _senceInfo._Weather));
    TableAddData(ui->tableWidget_SenceInfo, "天气", "", "目前的天气状态.", _weaterbox);

    QComboBox *_weatertime = new QComboBox();
    _weatertime->addItems(ConfigInfo::getInstance()->m_listTimeQuantum);

    _weatertime->setCurrentIndex(QStringListMatch(ConfigInfo::getInstance()->m_listTimeQuantum, _senceInfo._TimeQuantum));
    TableAddData(ui->tableWidget_SenceInfo, "时段", QString(""), QString("当前时间段."), _weatertime);

    TableAddData(ui->tableWidget_SenceInfo, "温度(℃)", QString::number(_senceInfo._Temp, 'f', 2), QString("当前环境温度."));
    TableAddData(ui->tableWidget_SenceInfo, "湿度(%rh)", QString::number(_senceInfo._Wet, 'f', 2), QString("当前环境湿度."));

    QComboBox *_weatersence = new QComboBox();
    _weatersence->addItems(ConfigInfo::getInstance()->m_listSence);
    _weatersence->setCurrentIndex(QStringListMatch(ConfigInfo::getInstance()->m_listSence, _senceInfo._Scene));
    TableAddData(ui->tableWidget_SenceInfo, "场景", QString(""), QString("当前运行场景."), _weatersence);
    QComboBox *_weaterpro[MAX_PANLE_NUM];
    for (auto i = 0; i < MAX_PANLE_NUM; i++)
    {
        _weaterpro[i] = new QComboBox();
        //        _weaterpro[i]->setEditable(true);
        _weaterpro[i]->addItems(ConfigInfo::getInstance()->m_listProject);
        _weaterpro[i]->setCurrentIndex(QStringListMatch(ConfigInfo::getInstance()->m_listProject, _senceInfo._Project[i]));
        QString str = QString("通道%1").arg(i + 1);
        TableAddData(ui->tableWidget_SenceInfo, str, QString(""), QString("通道对应的项目编号."), _weaterpro[i]);
    }
}

void SystemSetWidget::slot_SaveSenceParam_btClicked()
{
    CSenceInfo _senceInfo;
    readTableData(_senceInfo);
    ConfigInfo::getInstance()->SetSenceInfo(_senceInfo);
}

void SystemSetWidget::readTableData(CSenceInfo &info)
{

    int idx{0};
    QTableWidget *Tab = ui->tableWidget_SenceInfo;
    info._Time = Tab->item(idx++, 1)->text();
    info._Weather = dynamic_cast<QComboBox *>(Tab->cellWidget(idx++, 1))->currentText();
    info._TimeQuantum = dynamic_cast<QComboBox *>(Tab->cellWidget(idx++, 1))->currentText();
    info._Temp = Tab->item(idx++, 1)->text().toFloat();
    info._Wet = Tab->item(idx++, 1)->text().toFloat();
    info._Scene = dynamic_cast<QComboBox *>(Tab->cellWidget(idx++, 1))->currentText();
    for (auto i = 0; i < MAX_PANLE_NUM; i++)
    {
        info._Project[i] = dynamic_cast<QComboBox *>(Tab->cellWidget(idx++, 1))->currentText();
    }
}

#pragma endregion

#pragma region 保存路径配置
void SystemSetWidget::slot_browsePath_btnClicked()
{
    m_FilePath = QFileDialog::getExistingDirectory(this, "选择路径", "/media/xy/raid0");
    ui->lineEdit_savePath->setText(m_FilePath);
#ifdef __linux__
    MemCache::instance()->setValue("recordPath", m_FilePath);
#endif
    ConfigInfo::getInstance()->SetSavePath(m_FilePath);
    emit signal_sendSavePath(m_FilePath);
    QMessageBox::information(this, "提示", "设置成功");
}

void SystemSetWidget::slot_setPath_btnClicked()
{
//     m_FilePath = ui->lineEdit_savePath->text();
// #ifdef __linux__
//     MemCache::instance()->setValue("recordPath", m_FilePath);
// #endif
//     emit signal_sendSavePath(m_FilePath);
//     QMessageBox::information(this, "提示", "设置成功");
}

#pragma endregion

#pragma region 设备控制
void setMemCacheInfo(int idx, int typeId)
{
    ChannelInfo info = MemCache::instance()->getChannel(idx);
    info.chId = idx;
    info.typeId = typeId;
    MemCache::instance()->setValue(info);
}
QString getJson(QString content)
{
    QString ret{""};
    if (content.contains("XD01A_2"))
    {
    }
    else if (content.contains("XD01A_1"))
    {
        ret = "Gaode_ivi_1ch.json";
    }
    else if (content.contains("XD01B_2"))
    {
    }
    else if (content.contains("XD01B_1"))
    {
        ret = "alg_XD01B_1ch.json";
    }
    else if (content.contains("XH01A_2"))
    {
        ret = "Gaode_FIR_Camera_lowPixel_2ch.json";
    }
    else if (content.contains("XH01A_1"))
    {
        ret = "Gaode_FIR_Camera_lowPixel_1ch.json";
    }
    else if (content.contains("高德640x512_2"))
    {
        ret = "Gaode_FIR_Camera_2ch.json";
    }
    else if (content.contains("高德640x512_1"))
    {
        ret = "Gaode_FIR_Camera_1ch.json";
    }
    else if (content.contains("3840x2160_2"))
    {
    }
    else if (content.contains("3840x2160_1"))
    {
        ret = "alg_ox08b_1_ch.json";
    }
    else if (content.contains("1920x1280_1"))
    {
        // ret = "alg_ox03c_1_ch.json";
        ret = "alg_ox03c_1_ch_slave.json";
    }
    else if (content.contains("1920x1280_2"))
    {
        ret = "alg_ox03c_2_ch.json";
    }
    else if (content.contains("XP05_2"))
    {
        ret = "Gaode_FIR_1280x1024_Camera_2CH.json";
    }
    else if (content.contains("XP05_1"))
    {
        ret = "Gaode_FIR_1280x1024_Camera_1CH.json";
    }
    return ret;
}
void SystemSetWidget::btnConnectSlot(QTreeWidget *_treeWidget, QTreeWidgetItem *childItem, QString &&objName, QString btnName)
{
    QPushButton *btn = new QPushButton(btnName, this);
    _treeWidget->setItemWidget(childItem, 2, btn);
    btn->setObjectName(objName);
    btn->setMaximumSize(50,30);
    connect(btn, &QPushButton::clicked, this, &SystemSetWidget::slot_makeTag_btnClicked);
}
void SystemSetWidget::treeWidgetAddIterm_IIC_Camera(QTreeWidget *_treeWidget, QTreeWidgetItem *topItem, int &channel,CameraType type)
{
    // 创建配置文件子节点
    QTreeWidgetItem *childItem = new QTreeWidgetItem(QStringList{"配置文件", "", ""});
    // 将子节点添加到顶层节点上
    topItem->addChild(childItem);

    m_ComBox_jsonfile[channel] = new QComboBox();
    if(type == type_hw)
        m_ComBox_jsonfile[channel]->addItems(ConfigInfo::getInstance()->m_listIIC_JsonFile);
    else m_ComBox_jsonfile[channel]->addItems(ConfigInfo::getInstance()->m_listIIC_JsonFile_light);
    _treeWidget->setItemWidget(childItem, 1, m_ComBox_jsonfile[channel]);
    btnConnectSlot(_treeWidget, childItem, QString("writeJsonbtn%1").arg(channel));

    // 创建视频类型子节点
    QTreeWidgetItem *secChildItem = new QTreeWidgetItem(QStringList{"Y16/Y8", "", ""});
    topItem->addChild(secChildItem); // 将子节点添加到顶层节点上

    m_ComBox_videoType[channel] = new QComboBox();
    m_ComBox_videoType[channel]->addItems(ConfigInfo::getInstance()->m_listVideoType);
    _treeWidget->setItemWidget(secChildItem, 1, m_ComBox_videoType[channel]);
    btnConnectSlot(_treeWidget, secChildItem, QString("writeTypebtn%1").arg(channel));

    // 创建伪彩子节点
    QTreeWidgetItem *thrChildItem = new QTreeWidgetItem(QStringList{"伪彩类型", "", ""});
    topItem->addChild(thrChildItem); // 将子节点添加到顶层节点上

    m_ComBox_colorType[channel] = new QComboBox();
    m_ComBox_colorType[channel]->addItems(ConfigInfo::getInstance()->m_listColorType);
    _treeWidget->setItemWidget(thrChildItem, 1, m_ComBox_colorType[channel]);
    btnConnectSlot(_treeWidget, thrChildItem, QString("writeColorbtn%1").arg(channel));

    // 创建算法开关子节点
    QTreeWidgetItem *fourChildItem = new QTreeWidgetItem(QStringList{"算法开关", "", ""});
    topItem->addChild(fourChildItem); // 将子节点添加到顶层节点上

    m_CheckBox_algSwitch[channel] = new QCheckBox("开/关");
    _treeWidget->setItemWidget(fourChildItem, 1, m_CheckBox_algSwitch[channel]);
    m_CheckBox_algSwitch[channel]->setObjectName(QString("algSwitch%1").arg(channel));
    connect(m_CheckBox_algSwitch[channel], &QCheckBox::stateChanged, this, &SystemSetWidget::slot_makeTag_btnClicked);
    // btnConnectSlot(_treeWidget, fourChildItem, QString("writeAlgbtn%1").arg(channel));
}
void SystemSetWidget::treeWidgetAddIterm_Lidar(QTreeWidget *_treeWidget, QTreeWidgetItem *topItem, int &channel)
{
    // 创建激光子节点
    QTreeWidgetItem *ChildItem = new QTreeWidgetItem(QStringList{"IP地址", "", ""});
    topItem->addChild(ChildItem); // 将子节点添加到顶层节点上

    m_Edit_LidarIP[channel] = new QLineEdit(ConfigInfo::getInstance()->m_otherInfo.m_lidarIP);
    _treeWidget->setItemWidget(ChildItem, 1, m_Edit_LidarIP[channel]);

    btnConnectSlot(_treeWidget, ChildItem, QString("writelidarIPbtn%1").arg(channel));
}
void SystemSetWidget::treeWidgetAddIterm_Radar(QTreeWidget *_treeWidget, QTreeWidgetItem *topItem, int &channel)
{
    // 创建毫米波子节点
    QTreeWidgetItem *ChildItem = new QTreeWidgetItem(QStringList{"CAN端口", "", ""});
    topItem->addChild(ChildItem); // 将子节点添加到顶层节点上

    m_Edit_CanPort[channel] = new QLineEdit("0");
    _treeWidget->setItemWidget(ChildItem, 1, m_Edit_CanPort[channel]);

    btnConnectSlot(_treeWidget, ChildItem, QString("writecanPortbtn%1").arg(channel));
}
void SystemSetWidget::treeWidgetAddIterm_Net_Camera(QTreeWidget *_treeWidget, QTreeWidgetItem *topItem, int &channel)
{
    // 创建网络视频子节点
    QTreeWidgetItem *ChildItem = new QTreeWidgetItem(QStringList{"IP地址", "", ""});
    topItem->addChild(ChildItem); // 将子节点添加到顶层节点上

    m_Edit_CameraIP[channel] = new QLineEdit(ConfigInfo::getInstance()->m_otherInfo.m_CameraIP);
    _treeWidget->setItemWidget(ChildItem, 1, m_Edit_CameraIP[channel]);

    btnConnectSlot(_treeWidget, ChildItem, QString("writecameraIPbtn%1").arg(channel));


    // 创建视频类型子节点
    QTreeWidgetItem *secChildItem = new QTreeWidgetItem(QStringList{"Y16/Y8", "", ""});
    topItem->addChild(secChildItem); // 将子节点添加到顶层节点上

    m_ComBox_videoType[channel] = new QComboBox();
    m_ComBox_videoType[channel]->addItems(ConfigInfo::getInstance()->m_listVideoType);
    _treeWidget->setItemWidget(secChildItem, 1, m_ComBox_videoType[channel]);
    btnConnectSlot(_treeWidget, secChildItem, QString("writeTypebtn%1").arg(channel));

    // 创建伪彩子节点
    QTreeWidgetItem *thrChildItem = new QTreeWidgetItem(QStringList{"伪彩类型", "", ""});
    topItem->addChild(thrChildItem); // 将子节点添加到顶层节点上

    m_ComBox_colorType[channel] = new QComboBox();
    m_ComBox_colorType[channel]->addItems(ConfigInfo::getInstance()->m_listColorType);
    _treeWidget->setItemWidget(thrChildItem, 1, m_ComBox_colorType[channel]);
    btnConnectSlot(_treeWidget, thrChildItem, QString("writeColorbtn%1").arg(channel));

    // 创建显示类型子节点
    QTreeWidgetItem *fourChildItem = new QTreeWidgetItem(QStringList{"显示类型", "", ""});
    topItem->addChild(fourChildItem); // 将子节点添加到顶层节点上

    m_ComBox_disType[channel] = new QComboBox();
    m_ComBox_disType[channel]->addItems(ConfigInfo::getInstance()->m_listDisType);
    _treeWidget->setItemWidget(fourChildItem, 1, m_ComBox_disType[channel]);
    MemCache::instance()->setValue("Video_Dis_Type","IR");//默认显示红外视频
    btnConnectSlot(_treeWidget, fourChildItem, QString("writeDisTypebtn%1").arg(channel));

    // 创建融合切换子节点
    QTreeWidgetItem *fiveChildItem = new QTreeWidgetItem(QStringList{"是否融合", "", ""});
    topItem->addChild(fiveChildItem); // 将子节点添加到顶层节点上

    m_CheckBox_algSwitch[channel] = new QCheckBox("开/关");
    _treeWidget->setItemWidget(fiveChildItem, 1, m_CheckBox_algSwitch[channel]);
    // btnConnectSlot(_treeWidget, fiveChildItem, QString("writeAlgbtn%1").arg(channel));
    m_CheckBox_algSwitch[channel]->setObjectName(QString("algSwitch%1").arg(channel));
    connect(m_CheckBox_algSwitch[channel], &QCheckBox::stateChanged, this, &SystemSetWidget::slot_makeTag_btnClicked);
}
void SystemSetWidget::TreeWidgetInit()
{
    QSignalMapper *_Mapper = new QSignalMapper(this);
    QTreeWidget *_treeWidget = ui->treeWidget_deviceCtrl;
    _treeWidget->header()->setSectionResizeMode(QHeaderView::ResizeToContents);
    _treeWidget->setColumnCount(3);
    _treeWidget->setHeaderLabels(QStringList() << "通道号"
                                               << "内容"
                                               << "控制");
    _treeWidget->setColumnWidth(2,50);

    QTreeWidgetItem *newTopItem[MAX_ITEM_NUM];

    // 获取顶层节点
    for (int _channel = 0; _channel < MAX_ITEM_NUM; _channel++)
    {
        QString str_channel = QString("通道%1").arg(_channel + 1);
        QList<QString> ItemName = {str_channel, "", ""};
        // 创建顶层Item
        newTopItem[_channel] = new QTreeWidgetItem(ItemName);
        _treeWidget->addTopLevelItem(newTopItem[_channel]);
        if (m_ComBox_item_Type[_channel] == nullptr)
        {
            m_ComBox_item_Type[_channel] = new QComboBox();
            connect(m_ComBox_item_Type[_channel], SIGNAL(currentIndexChanged(int)), _Mapper, SLOT(map()));
            _Mapper->setMapping(m_ComBox_item_Type[_channel], _channel);
        }
        m_ComBox_item_Type[_channel]->addItems(ConfigInfo::getInstance()->m_listItemType);
        _treeWidget->setItemWidget(newTopItem[_channel], 1, m_ComBox_item_Type[_channel]);

        m_topItem[_channel] = _treeWidget->topLevelItem(_channel);
        m_topItem[_channel] = _treeWidget->topLevelItem(_channel);
        treeWidgetAddIterm_IIC_Camera(_treeWidget, m_topItem[_channel], _channel);
        setMemCacheInfo(_channel, TYPEI2CDIRECT);
    }
    connect(_Mapper, SIGNAL(mapped(int)), this, SLOT(slot_ComBox_Change_ItemType(int)));

    // 数据添加完成，保证所有数据都是展开形状
    _treeWidget->setItemsExpandable(true); // 子Item是否可以被展开
    _treeWidget->expandAll();
}

void SystemSetWidget::slot_ComBox_Change_ItemType(int idx)
{
    QTreeWidget *_treeWidget = ui->treeWidget_deviceCtrl;
    QString _content = m_ComBox_item_Type[idx]->currentText();
    if (_content.contains("IIC_"))
    {
        setMemCacheInfo(idx, TYPEI2CDIRECT);
        m_topItem[idx] = _treeWidget->topLevelItem(idx);
        while (m_topItem[idx]->childCount())
        {
            QTreeWidgetItem *child = m_topItem[idx]->takeChild(0);
            delete child;
        }
        CameraType type = _content.contains("可见光") == true ? type_light : type_hw;
        treeWidgetAddIterm_IIC_Camera(_treeWidget, m_topItem[idx], idx,type);
    }
    else if (_content == "网络相机")
    {
        m_topItem[idx] = _treeWidget->topLevelItem(idx);
        while (m_topItem[idx]->childCount())
        {
            QTreeWidgetItem *child = m_topItem[idx]->takeChild(0);
            delete child;
        }
        treeWidgetAddIterm_Net_Camera(_treeWidget, m_topItem[idx], idx);
    }
    else if (_content == "激光雷达")
    {
        m_topItem[idx] = _treeWidget->topLevelItem(idx);
        while (m_topItem[idx]->childCount())
        {
            QTreeWidgetItem *child = m_topItem[idx]->takeChild(0);
            delete child;
        }
        treeWidgetAddIterm_Lidar(_treeWidget, m_topItem[idx], idx);
    }
    else if (_content == "毫米波雷达")
    {
        m_topItem[idx] = _treeWidget->topLevelItem(idx);
        while (m_topItem[idx]->childCount())
        {
            QTreeWidgetItem *child = m_topItem[idx]->takeChild(0);
            delete child;
        }
        treeWidgetAddIterm_Radar(_treeWidget, m_topItem[idx], idx);
    }
    _treeWidget->setItemsExpandable(false); // 子Item是否可以被展开
    _treeWidget->setItemsExpandable(true);  // 子Item是否可以被展开
    _treeWidget->expandAll();
}

void SystemSetWidget::slot_makeTag_btnClicked()
{
    QPushButton *btn = (QPushButton *)sender();
    for (auto channel = 0; channel < MAX_ITEM_NUM; channel++)
    {
#pragma region 更改配置文件
        if (btn->objectName() == QString("writeJsonbtn%1").arg(channel))
        {
            ChannelInfo info = MemCache::instance()->getChannel(channel);
            info.chId = channel;
            info.typeId = TYPEI2CDIRECT;

            QString jsonFile = getJson(m_ComBox_jsonfile[channel]->currentText());
            if (!jsonFile.isEmpty())
            {
                CInteractionByPcie::instance()->sendMsgByConsole(jsonFile, channel / 2, channel);
            }
            else
            {
                QMessageBox::information(NULL, "提示", "未找到Json配置文件");
            }
        }
#pragma endregion 

#pragma region 视频类型设置
        else if (btn->objectName() == QString("writeTypebtn%1").arg(channel)) {
            QString _content = m_ComBox_item_Type[channel]->currentText();
            if(_content.contains("网络相机")){
                QString videoType = m_ComBox_videoType[channel]->currentText();
                bool bRet;
                QString _ip = m_Edit_CameraIP[channel]->text();
                if (videoType.contains("Y8"))
                    bRet = ApiHandler::instance()->setFormat(1,_ip);
                else if(videoType.contains("Y16"))
                    bRet = ApiHandler::instance()->setFormat(3,_ip);
                else{}
            }else if (_content.contains("可见光")){
                QString videoType = m_ComBox_videoType[channel]->currentText();
                QString val;
                if (videoType.contains("H264")) {
                    val = "h264";
                } else {
                    val = "yuv";
                }

                MemCache::instance()->setValue("light_videoType", val);
            } else {
                if (m_ComBox_jsonfile[channel]->currentText().contains("艾利光")){
                    Log::Logger->warn("艾利光无法切换视频格式.");
                    return;
                }
                uint32_t readCmd = 0x1632;
                if (m_ComBox_jsonfile[channel]->currentText().contains("XP05")){
                    readCmd = 0x1616;
                }
                QString videoType = m_ComBox_videoType[channel]->currentText();
                int cmd{0};
                if (videoType.contains("Y16"))
                    cmd = 0x03;
                else
                    cmd = 0x01;

                bool ret{false};
                if (!CInteractionByPcie::instance()->writeReg(0, channel, 0x66, 0x140, cmd, 0x1616))
                {
                    Log::Logger->warn("video type switch:Write 0x140 0x03 fail.");
                    QMessageBox::information(NULL, "提示", "视频格式切换失败");
                    return;
                }

                for (int foo = 0; foo < 3; foo++)
                {
                    if (!CInteractionByPcie::instance()->readReg(0, channel, 0x66, 0x140, readCmd, cmd))
                    {
                        qDebug() << ">>>>>>>>>>>>>>> Read 0x140 0x03 FAIL";
                        Log::Logger->warn("video type switch:Read 0x140 0x03 fail.");
                    }
                    else
                    {
                        QMessageBox::information(NULL, "提示", "视频格式切换成功");
                        break;
                    }
                    QThread::msleep(500);
                }
            }
            
        }
 #pragma endregion

#pragma region 伪彩类型设置
        else if (btn->objectName() == QString("writeColorbtn%1").arg(channel)) {
            QString _content = m_ComBox_item_Type[channel]->currentText();
            if(_content.contains("网络相机")){
                QString curColor = m_ComBox_colorType[channel]->currentText();
                bool bRet;
                QString _ip = m_Edit_CameraIP[channel]->text();
                if (curColor.contains("白热"))
                {
                    bRet = ApiHandler::instance()->setColor(0,_ip);
                }
                else if (curColor.contains("铁红"))
                {
                    bRet = ApiHandler::instance()->setColor(2,_ip);
                }
                else if (curColor.contains("绿热"))
                {
                    bRet = ApiHandler::instance()->setColor(0x15,_ip);
                }
                else if (curColor.contains("蓝热"))
                {
                    bRet = ApiHandler::instance()->setColor(0x16,_ip);
                }
                else return;

            }else{
                if (m_ComBox_jsonfile[channel]->currentText().contains("艾利光"))
                {
                    Log::Logger->warn("艾利光无法切换伪彩.");
                    return;
                }
                uint32_t readCmd = 0x1632;
                if (m_ComBox_jsonfile[channel]->currentText().contains("XP05"))
                {
                    readCmd = 0x1616;
                }
                int cmd = 0;
                QString curColor = m_ComBox_colorType[channel]->currentText();
                if (curColor.contains("白热"))
                {
                    cmd = 0;
                }
                else if (curColor.contains("铁红"))
                {
                    cmd = 12; // caihong
                }
                else if (curColor.contains("绿热"))
                {
                    cmd = 21; // lvre
                }
                else if (curColor.contains("蓝热"))
                {
                    cmd = 22; // lanre
                }
                else
                    return;

                if (!CInteractionByPcie::instance()->writeReg(0, channel, 0x66, 0x200, cmd, 0x1616))
                {
                    Log::Logger->warn("color switch:Write 0x200 value fail.");
                }
                if (!CInteractionByPcie::instance()->readReg(0, channel, 0x66, 0x200, readCmd, cmd))
                {
                    Log::Logger->warn("color switch:Read 0x200 value fail.");
                    QMessageBox::information(NULL, "提示", "伪彩切换失败");
                }
                else
                {
                    QMessageBox::information(NULL, "提示", "伪彩切换成功");
                }
            }
        }
#pragma endregion

#pragma region 网络相机视频显示设置
        else if (btn->objectName() == QString("writeDisTypebtn%1").arg(channel))
        {
            QString _disType = m_ComBox_disType[channel]->currentText();
            if (_disType.contains("可见光")) {
                MemCache::instance()->setValue("Video_Dis_Type","VL");
            }else if(_disType.contains("红外")){
                MemCache::instance()->setValue("Video_Dis_Type","IR");
            }
        }
#pragma endregion

#pragma region 算法切换
        else if (btn->objectName() == QString("algSwitch%1").arg(channel)) {
            QString _content = m_ComBox_item_Type[channel]->currentText();
            bool _algSwitch = m_CheckBox_algSwitch[channel]->isChecked();
            if(_content.contains("网络相机")){
                bool bRet;
                QString _ip = m_Edit_CameraIP[channel]->text();
                bRet = ApiHandler::instance()->setFusionSwitch(_algSwitch,_ip);
                if(_algSwitch) m_CheckBox_algSwitch[channel]->setText("开");
                else m_CheckBox_algSwitch[channel]->setText("关");
                QMessageBox::information(NULL, "提示", "算法切换成功");
            }else{
                if (m_ComBox_jsonfile[channel]->currentText().contains("艾利光")) {
                    Log::Logger->warn("艾利光无法切换算法.");
                    return;
                }
                uint32_t readCmd = 0x1632;
                if (m_ComBox_jsonfile[channel]->currentText().contains("XP05")) {
                    readCmd = 0x1616;
                }
                int cmd = m_CheckBox_algSwitch[channel]->isChecked();
                if(cmd) m_CheckBox_algSwitch[channel]->setText("开");
                else m_CheckBox_algSwitch[channel]->setText("关");
                if (!CInteractionByPcie::instance()->writeReg(0, channel, 0x66, 0x400, cmd, 0x1616)) {
                    Log::Logger->warn("alg switch:Write 0x200 value fail.");
                }
                CInteractionByPcie::instance()->readReg(0, channel, 0x66, 0x400, readCmd, cmd);
                QMessageBox::information(NULL, "提示", "算法切换成功");
                // if (!CInteractionByPcie::instance()->readReg(0, channel, 0x66, 0x400, readCmd, cmd))
                // {
                //     QMessageBox::information(NULL, "提示", "算法切换失败");
                // }
                // else
                // {
                //     QMessageBox::information(NULL, "提示", "算法切换成功");
                // }
            }
        }
#pragma endregion

#pragma region 算法切换-已弃用
        else if (btn->objectName() == QString("writeAlgbtn%1").arg(channel))
        {
            if (m_ComBox_jsonfile[channel]->currentText().contains("艾利光"))
            {
                Log::Logger->warn("艾利光无法切换算法.");
                return;
            }
            uint32_t readCmd = 0x1632;
            if (m_ComBox_jsonfile[channel]->currentText().contains("XP05"))
            {
                readCmd = 0x1616;
            }
            int cmd = m_CheckBox_algSwitch[channel]->isChecked();

            if (!CInteractionByPcie::instance()->writeReg(0, channel, 0x66, 0x400, cmd, 0x1616))
            {
                Log::Logger->warn("alg switch:Write 0x200 value fail.");
            }

            if (!CInteractionByPcie::instance()->readReg(0, channel, 0x66, 0x400, readCmd, cmd))
            {
                QMessageBox::information(NULL, "提示", "算法切换失败");
            }
            else
            {
                QMessageBox::information(NULL, "提示", "算法切换成功");
            }
        }
#pragma endregion

#pragma region 激光雷达初始化
        else if (btn->objectName() == QString("writelidarIPbtn%1").arg(channel))
        {

            emit signal_lidar_init(m_Edit_LidarIP[channel]->text());
        }
#pragma endregion

#pragma region 毫米波雷达初始化
        else if (btn->objectName() == QString("writecanPortbtn%1").arg(channel))
        { // Switch video types by IIC
            emit signal_radar_init(m_Edit_CanPort[channel]->text().toInt());
        }
#pragma endregion

#pragma region  网络相机初始化
        else if (btn->objectName() == QString("writecameraIPbtn%1").arg(channel))
        { // Switch video types by IIC
            QString _ip = m_Edit_CameraIP[channel]->text();
            qDebug() << "ip:" << _ip; 
            if (!m_artosVideo[channel])
            {
                m_artosVideo[channel] = new ArtosynNet(channel);
            }
            if (!m_artosVideo[channel]->netConnect(_ip))
            {
                QMessageBox::information(this, "提示", "配置失败");
            }
            else
            {
                ChannelInfo info = MemCache::instance()->getChannel(channel);
                info.chId = channel;
                info.typeId = TYPEARTOSYN;

                MemCache::instance()->setValue(info);
                QMessageBox::information(this, "提示", "Connect OK");
                MemCache::instance()->setValue("video" + QString::number(channel), "1");

                bool bRet = ApiHandler::instance()->setTime();
                if(!bRet)  QMessageBox::information(this, "提示", "网络相机授时失败.");
            }
        }
#pragma endregion

    }
}

#pragma endregion

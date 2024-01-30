#include "transconfig.h"
#include "ui_transconfig.h"
#include <QCoreApplication>
#include <QDir>
#include <QFileInfoList>
#include <QDebug>
#include <QProcess>
#include "memcache.h"
#include <QSignalMapper>
#include <QLineEdit>
#include <QMessageBox>
#include <QThread>
#include <thread>
#include <QElapsedTimer>
#include <QPushButton>
#include <alg_sdk/service.h>

bool getI2cAddr(uint8_t board_id, uint8_t ch_id)
{
    const char *topic_name = "/service/i2c/info";
    service_board_i2c_info_get_t  t = {
        .ack_mode = 1,
        .board_id = 0,
        .channel_num = 0,
    };
    t.board_id = board_id;
    t.channel_num = ch_id;
    for (int foo = 0; foo < 5; foo++ ) {
        int rc = alg_sdk_call_service(topic_name, &t, 10000);
        if (rc < 0)
        {
            qDebug( "Request Service : [%s] Error!\n", topic_name);
            return false;
        }

        qDebug("[ack : %d]\n", t.ack_code);
        qDebug("i2c_dev_cnt: %d\r\n",t.i2c_addr_dev_cnt);
        for (int i = 0; i < t.i2c_addr_dev_cnt; i++)
        {
            printf("i2c_dev_addr[%d]:0x%x\r\n",i,t.i2c_addr_array[i]);
        }

        if (t.i2c_addr_dev_cnt >= 3) return true;

        QThread::msleep(300);
    }

    return false;
}

// ./pcie_sdk_demo_service -write_reg 0 4 0x66 0x140 0x19 0x1616
// board id
// channel
// addr
// reg addr
// value
// bytes
bool readReg(uint8_t board_id, uint8_t ch_id, uint16_t i2c_add, uint16_t reg_addr, uint16_t type, unsigned char check)
{
    /* Example : Read IIC */
    const char *topic_name = "/service/camera/read_reg";

    service_camera_read_reg_t t1 = {
        .ack_mode = 1,
        .ch_id = 0,
        .msg_type = 0x1608,
        .device_addr = 0x90,
        .line_len = 1,
    };
    uint16_t reg[1] = {0};
    // board_id = strtol(argv[2],NULL,16);
    // ch_id = strtol(argv[3],NULL,16);
    t1.ch_id = board_id*8 + ch_id;
    t1.device_addr = i2c_add;
    reg[0] = reg_addr;
    t1.msg_type = type;

    qDebug("read: ch[%d]: dev_addr:0x%x,reg:0x%x,fmt:0x%x\r\n",t1.ch_id,t1.device_addr,reg[0],t1.msg_type);
    for (int i = 0; i < t1.line_len; i++)
    {
        t1.payload[i] = reg[i];
    }
    int rc = alg_sdk_call_service(topic_name, &t1, 10000);
    if (rc < 0)
    {
        qDebug( "Request Service : [%s] Error!\n", topic_name);
        return false;
    }

    qDebug("[ack : %d], [channel : %d]\n", t1.ack_code, t1.channel);
    if(0 != t1.ack_code)
    {
        qDebug("[Error]: IIC Read Failed! Check Dev_addr or Reg is exit?\r\n");
        return false;
    }
    for (int i = 0; i < t1.length_r; i++)
    {
        qDebug("[len : %d], [read_data[%d] : 0x%X]\n", t1.length_r, i,t1.data[i]&0xff);
    }

    if ((unsigned char)(t1.data[0] & 0xff )== check) {
        qDebug() << ">>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>ok<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<";
        qDebug()  << (unsigned char)(t1.data[0] & 0xff );
        return true;
    } else {
        qDebug() << ">>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>fail<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<";
        qDebug()  << (unsigned char)(t1.data[0] & 0xff );
    }

    return false;
    /* End */
}
bool writeReg(uint8_t board_id, uint8_t ch_id, uint8_t i2c_add, uint16_t reg_addr, uint8_t value, uint16_t type)
{

    /* Example : Write IIC */
    const char *topic_name = "/service/camera/write_reg";

    service_camera_write_reg_t t1 = {
        .ack_mode = 1,
        .ch_id = 0,
        .msg_type = 0x1608,
        .device_addr = 0x80,
        .line_len = 2,
    };
    uint16_t reg[2] = {0};
    //board_id = strtol(argv[2],NULL,16);
    //ch_id = strtol(argv[3],NULL,16);
    t1.ch_id = board_id*8 + ch_id;
    //t1.device_addr = strtol(argv[4],NULL,16);
    t1.device_addr = i2c_add;
    //reg[0] = strtol(argv[5],NULL,16);
    reg[0] = reg_addr;
    //reg[1] = strtol(argv[6],NULL,16);
    reg[1] = value;
    //t1.msg_type = strtol(argv[7],NULL,16);
    t1.msg_type = type;
    qDebug("write: ch[%d]: dev_addr:0x%x, reg:0x%x, data:0x%x, fmt:0x%x\r\n",t1.ch_id,t1.device_addr,reg[0],reg[1],t1.msg_type);

    for (int i = 0; i < t1.line_len; i++)
    {
        t1.payload[i] = reg[i];
    }

    int rc = alg_sdk_call_service(topic_name, &t1, 10000);
    if (rc < 0)
    {
        qDebug( "Request Service : [%s] Error!\n", topic_name);
        return false;
    }

    printf("[ack : %d], [channel : %d]\n", t1.ack_code, t1.channel);
    if(0 != t1.ack_code)
    {
        qDebug("[Error]:IIC Write Failed! Check Dev_addr or Reg is exit?\r\n");
        return false;
    }

    return true;
    /* End */
}

TransConfig::TransConfig(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::TransConfig)
{
    ui->setupUi(this);
    QString sAppPath = QCoreApplication::applicationDirPath();
    QString sConfigPath = sAppPath + "/config";
    QDir dir(sConfigPath);
    dir.setFilter(QDir::Files | QDir::Hidden | QDir::NoSymLinks);
    dir.setSorting(QDir::Size | QDir::Reversed);

    QFileInfoList list = dir.entryInfoList(); //获取文件夹中的文件信息列表
    qInfo() << "Bytes Filename" ;
    for (int i = 0; i < list.size(); ++i)
    {
        QFileInfo fileInfo = list.at(i); //获取文件列表中某个文件的信息
        if(fileInfo.fileName().contains("json")) {
            m_configList.append(fileInfo.fileName());
        }
        qInfo()<<fileInfo.path();
    }
    QComboBox * cb[9] = {ui->cb1,ui->cb2,ui->cb3,ui->cb4,ui->cb5,
                        ui->cb6,ui->cb7, ui->cb8,ui->cb9};
    QSignalMapper * mapper = new QSignalMapper(this);
    m_mapper = new QSignalMapper(this);
    m_mapperLine = new QSignalMapper(this);
    m_mapModeSwitch = new QSignalMapper(this);
    m_mapWC = new QSignalMapper(this);
    m_mapAlgo = new QSignalMapper(this);
    for(int i = 0;i < 9; i ++) {
        cb[i]->insertItem(TYPENONE, "NONE");
        cb[i]->insertItem(TYPEI2CDIRECT, "I2C直连");
        cb[i]->insertItem(TYPEI2CDIRECT, "Artosyn");
        cb[i]->insertItem(TYPEJIGUANG, "激光雷达");
        connect(cb[i], SIGNAL(currentIndexChanged(int)), mapper, SLOT(map()));
        mapper->setMapping(cb[i], cb[i]);
    }
    connect(mapper, SIGNAL(mapped(QWidget*)), this, SLOT(on_connectTypeChanged(QWidget*)));
}

TransConfig::~TransConfig()
{
    delete ui;
}


void TransConfig::on_connectTypeChanged(QWidget *w)
{
    QComboBox *cur = (QComboBox *)w;
    qInfo()<< cur->currentText();
    qInfo()<< cur->currentIndex();
    int idxRow = getRowIdx(w, 1);
    qInfo() << "index Row: " << idxRow;

    int typeId = 0;
    if (cur->currentText().contains("I2C")) {
        typeId = TYPEI2CDIRECT;
    } else if (cur->currentText().contains("Artosyn")) {
        typeId = TYPEARTOSYN;
    } else {
        typeId = 0;
        return;
    }

    {
        ChannelInfo info = MemCache::instance()->getChannel(idxRow);
        info.chId = idxRow;
        info.typeId = typeId;
        MemCache::instance()->setValue(info);
    }

    emit sigUiChange(idxRow);
    removeItem(idxRow);
    if(cur->currentText().contains("NONE")) {
//        MemCache::instance()->setValue("video"+QString::number(idxRow), "0");
//        removeItem(idxRow);
    }else if(cur->currentText().contains("I2C")) {
        for (int foo  = 2; foo < 6; foo++) {
            QLayoutItem * item = ui->gridLayout->itemAtPosition(idxRow, foo);
            if (item) {
                ui->gridLayout->removeItem(item);
            }
        }

        // 添加通道选择器
        // ui->gridLayout->addWidget(new QLabel("组号选择"), idxRow, 2);
        // QComboBox * box = new QComboBox();
        // box->insertItem(0, "0组");
        // box->insertItem(1, "1组");
        // box->insertItem(2, "2组");
        // box->insertItem(3, "3组");
        // ui->gridLayout->addWidget(box, idxRow, 3);

        // 添加配置文件选择
        //ui->gridLayout->addWidget(new QLabel("协议选择"), idxRow, 3);
        QComboBox * box = new QComboBox();
        box->insertItem(0, "NONE");
        for(int i = 0;i < m_configList.size(); i ++) {
            box->insertItem(i + 1, m_configList[i]);
        }
        ui->gridLayout->addWidget(box, idxRow, 3);

        // 绑定配置文件ComboBox信号槽
        connect(box, SIGNAL(currentIndexChanged(int)), m_mapper, SLOT(map()));
        m_mapper->setMapping(box, box);
        disconnect(m_mapper, SIGNAL(mapped(QWidget*)), this, SLOT(on_connectProChanged(QWidget*)));
        connect(m_mapper, SIGNAL(mapped(QWidget*)), this, SLOT(on_connectProChanged(QWidget*)));


        QPushButton * btn = new QPushButton();
        ui->gridLayout->addWidget(btn, idxRow, 4);
        btn->setText("Switch Y16");
        m_mapModeSwitch->setMapping(btn, btn);
        connect(btn, SIGNAL(clicked(bool)), m_mapModeSwitch, SLOT(map()));
        disconnect(m_mapModeSwitch, SIGNAL(mapped(QWidget*)), this, SLOT(onBtnClicked(QWidget*)));
        connect(m_mapModeSwitch, SIGNAL(mapped(QWidget*)), this, SLOT(onBtnClicked(QWidget*)));

        QComboBox * boxWC = new QComboBox();
        boxWC->insertItem(0, "伪彩类型");
        boxWC->insertItem(1, "白热");
        boxWC->insertItem(2, "蓝热");
        boxWC->insertItem(3, "绿热");
        boxWC->insertItem(4, "铁红");
        ui->gridLayout->addWidget(boxWC, idxRow, 5);
        connect(boxWC, SIGNAL(currentIndexChanged(int)), m_mapWC, SLOT(map()));
        m_mapWC->setMapping(boxWC, boxWC);
        disconnect(m_mapWC, SIGNAL(mapped(QWidget*)), this, SLOT(onWCEditChanged(QWidget*)));
        connect(m_mapWC, SIGNAL(mapped(QWidget*)), this, SLOT(onWCEditChanged(QWidget*)));

        QComboBox * boxAlgo = new QComboBox();
        boxAlgo->insertItem(0, "算法开关");
        boxAlgo->insertItem(1, "一键开算法");
        boxAlgo->insertItem(2, "一键关算法");
        ui->gridLayout->addWidget(boxAlgo, idxRow, 6);
        connect(boxAlgo, SIGNAL(currentIndexChanged(int)), m_mapAlgo, SLOT(map()));
        m_mapAlgo->setMapping(boxAlgo, boxAlgo);
        disconnect(m_mapAlgo, SIGNAL(mapped(QWidget*)), this, SLOT(onAlgoEditChanged(QWidget*)));
        connect(m_mapAlgo, SIGNAL(mapped(QWidget*)), this, SLOT(onAlgoEditChanged(QWidget*)));
        
        MemCache::instance()->setValue("video"+QString::number(idxRow), "1");
    } else if (cur->currentText().contains("Artosyn")) {
        m_artosVideo = new ArtosynNet(idxRow);
        //new std::thread([this](int id){
        if (!m_artosVideo->netConnect("192.168.1.10", 6000)) {
            QMessageBox::information(this, "提示", "配置失败");
        } else {
            QMessageBox::information(this, "提示", "Connect OK");
            MemCache::instance()->setValue("video"+QString::number(idxRow), "1");
        }
        //}, idxRow);
    } else {
        MemCache::instance()->setValue("video"+QString::number(idxRow), "1");
        ui->gridLayout->addWidget(new QLabel("IP地址"), idxRow, 2);
        QLineEdit * pLine = new QLineEdit("0.0.0.0");
        ui->gridLayout->addWidget(pLine, idxRow, 3);
        connect(pLine, SIGNAL(returnPressed()), m_mapperLine, SLOT(map()));
        m_mapperLine->setMapping(pLine, pLine);
        disconnect(m_mapperLine, SIGNAL(mapped(QWidget*)), this, SLOT(on_connectEditChanged(QWidget*)));
        connect(m_mapperLine, SIGNAL(mapped(QWidget*)), this, SLOT(on_connectEditChanged(QWidget*)));
    }
}

// close/open algorithm
void TransConfig::onAlgoEditChanged(QWidget *w)
{
    QComboBox * cur = (QComboBox *) w;
    int iChannel = getRowIdx(w, 6);


    ChannelInfo ci = MemCache::instance()->getChannel(iChannel);
    unsigned int reg_format = 0x1632;
    if (ci.proId == 38) {
        reg_format = 0x1616;
    }

    qDebug() << "Current Channel Index: " << iChannel;
    qDebug() << "Current Text: " << cur->currentText();
    int value = 0;
    if (cur->currentText().contains("一键开算法")) {
        value = 1;
    } else if (cur->currentText().contains("一键关算法")) {
        value = 0;
    } else {
        return;
    }

    // 0x400 write only
    if (!readReg(0, iChannel, 0x66, 0x400, reg_format, value)) {
        qDebug() << ">>>>>>>>>>>>>>> Read 0x200 value FAIL";
    } 
    if (!writeReg(0, iChannel, 0x66, 0x400, value, 0x1616)) {
        qDebug() << ">>>>>>>>>>>>>>> Write 0x200 value FAIL";
    }

    QMessageBox::information(this, "nullptr", "OK");
}

void TransConfig::onWCEditChanged(QWidget *w)
{
    QComboBox * cur = (QComboBox *) w;

    // if(cur->currentIndex() == 0) {
    //     return;
    // }

    // 通道号
    int iChannel = getRowIdx(w, 5);
    // QLayoutItem *item3 = ui->gridLayout->itemAtPosition(iChannel, 3);
    // if (!item3) {
    //     QMessageBox::information(this, "null", "get page handle failed");
    //     return;
    // }
    // QComboBox * curItem = (QComboBox *)item3->widget();
    // int iGroup = iChannel / 2;
    
    qDebug() << "Current Channel Index: " << iChannel;
    qDebug() << "Current Text: " << cur->currentText();
    
    ChannelInfo ci = MemCache::instance()->getChannel(iChannel);
    unsigned int readFormat = 0x1632;
    if (ci.proId == 38) {
        readFormat = 0x1616;
    }

    int value = 0;
    if (cur->currentText().contains("白热")) {
        value = 0;
    } else if (cur->currentText().contains("铁红")) {
        value = 12;//caihong
    } else if (cur->currentText().contains("绿热")) {
        value = 21;//lvre
    } else if (cur->currentText().contains("蓝热")) {
        value = 22;//lanre
    } else {
        return;
    }
    if (!readReg(0, iChannel, 0x66, 0x200, readFormat, value)) {
        qDebug() << ">>>>>>>>>>>>>>> Read 0x200 value FAIL";
    } 
    if (!writeReg(0, iChannel, 0x66, 0x200, value, 0x1616)) {
        qDebug() << ">>>>>>>>>>>>>>> Write 0x200 value FAIL";
    }
    if (!readReg(0, iChannel, 0x66, 0x200, readFormat, value)) {
        qDebug() << ">>>>>>>>>>>>>>> Read 0x200 value FAIL";
    } else {
        QMessageBox::information(this, "nullptr", "OK");
        return;
    }

    QMessageBox::information(this, "nullptr", "FAIL");
}

void TransConfig::onBtnClicked(QWidget *w)
{
    QPushButton* btn = (QPushButton*)w;
    int iChannel = getRowIdx(w, 4);
    qDebug() << "Current Channel Index: " << iChannel;
    QLayoutItem *item = ui->gridLayout->itemAtPosition(iChannel, 1);

    ChannelInfo ci = MemCache::instance()->getChannel(iChannel);
    unsigned int readFormat = 0x1632;
    if (ci.proId == 38) {
        readFormat = 0x1616;
    }

    bool brst = false;
    if (btn->text().contains("Y16")) {
        brst = false;
        for (int foo = 0; foo < 3; foo++) {
            if (writeReg(0, iChannel, 0x66, 0x140, 0x03, 0x1616)) {
                brst = true;
                break;
            }
            qDebug() << ">>>>>>>>>>>>>>> Write 0x140 0x03 FAIL";
            QThread::msleep(1000);
        }
        if (brst) {
            brst = false;
            QThread::msleep(2000);
            for (int foo = 0; foo < 3; foo++) {
                if (!readReg(0, iChannel, 0x66, 0x140, readFormat, 0x03)) {
                    qDebug() << ">>>>>>>>>>>>>>> Read 0x140 0x03 FAIL";
                } else {
                    brst = true;
                    btn->setText("Switch Y8");
                    break;
                }
                QThread::msleep(1000);
            }
        }
    } else {
        brst = false;
        for (int foo = 0; foo < 3; foo++) {
            if (writeReg(0, iChannel, 0x66, 0x140, 0x01, 0x1616)) {
                brst = true;
                break;
            }
            qDebug() << ">>>>>>>>>>>>>>> Write 0x140 0x01 FAIL";
            QThread::msleep(1000);
        }
        if (brst) {
            QThread::msleep(2000);
            brst = false;
            for (int foo = 0; foo < 3; foo++) {
                if (!readReg(0, iChannel, 0x66, 0x140, readFormat, 0x01)) {
                    qDebug() << ">>>>>>>>>>>>>>> Read 0x140 0x01 FAIL";
                } else {
                    brst = true;
                    btn->setText("Switch Y16");
                    break;
                }
                QThread::msleep(1000);
            }
        }
    }

    if (brst) {
        QMessageBox::information(this, "nullptr", "OK");
    } else {
        QMessageBox::information(this, "nullptr", "FAIL");
    }
}

void TransConfig::on_connectProChanged(QWidget *w)
{
    QComboBox * cur = (QComboBox *) w;

    // if(cur->currentIndex() == 0) {
    //     return;
    // }

    // 通道号
    int iChannel = getRowIdx(w, 3);
    qDebug() << "Current Channel Index: " << iChannel;
    QLayoutItem *item = ui->gridLayout->itemAtPosition(iChannel, 1);
    if(!item) {
         QMessageBox::information(this, "提示", "未知错误");
         return;
    }

    QComboBox * curItem = (QComboBox *)item->widget();

    int iGroup = iChannel / 2;
    qDebug() << "Current Channel: " << iChannel;
    qDebug() << "Current Group: " << iGroup;
    qDebug() << curItem->currentText();

    int typeId = 0;
    if (curItem->currentText().contains("I2C")) {
        typeId = TYPEI2CDIRECT;
    } else if (curItem->currentText().contains("Artosyn")) {
        typeId = TYPEARTOSYN;
    } else {
        typeId = 0;
        return;
    }

    {
        ChannelInfo info;
        info.chId = iChannel % 9;
        info.typeId = typeId;
        info.tChanid = iGroup;
        // info.proId = cur->currentIndex();
        MemCache::instance()->setValue(info);
    }

    if (typeId == TYPEI2CDIRECT) {
        QString sAppPath = QCoreApplication::applicationDirPath();
        QString spythonPath = sAppPath + "/python";
        m_pythonProcess.start("bash");
        m_pythonProcess.waitForStarted();
        QString sCmd = "cd  "+spythonPath+"\n";
        qInfo()<<sCmd;
        m_pythonProcess.write(sCmd.toStdString().data());

        sCmd = "python set_sensor_from_json.py --json_file='" + sAppPath + "/config/" +
                cur->currentText() + "' --channel_id=" + QString::number(iGroup) + "\n";
        qInfo()<<sCmd;
        bool bRst = false;
        int icount = 0;
        for (int foo = 0; foo < 3; foo++) {
            m_pythonProcess.write(sCmd.toStdString().data());
            m_pythonProcess.waitForBytesWritten(10000);
            // m_pythonProcess.waitForFinished(10000);
            QElapsedTimer timer;
            timer.start();
            QByteArray outStd;
            while (timer.elapsed() < 20000) {
                QCoreApplication::processEvents();
                outStd += m_pythonProcess.readAll();
                if (outStd.contains("result =")) {
                    bRst = true;
                    break;
                }
                QThread::msleep(100);
            }
            qDebug() << outStd;
            if (bRst) {
                break;
            } 
            QThread::msleep(200);
        }

        // m_pythonProcess.write("exit\n");
        if (bRst) {
            sCmd = "python " + sAppPath + "/python/stream_on_by_channel.py --channel=" + QString::number(iChannel) + "\n";
            qInfo()<<sCmd;
            bRst = false;
            QThread::msleep(1000);
            int icount = 0;
            for (int foo = 0; foo < 3; foo++) {
                m_pythonProcess.write(sCmd.toStdString().data());
                m_pythonProcess.waitForBytesWritten(10000);
                QElapsedTimer timer;
                timer.start();
                QByteArray outStd;
                while (timer.elapsed() < 10000) {
                    QCoreApplication::processEvents();
                    outStd += m_pythonProcess.readAll();
                    if (outStd.contains("result = 0")) {
                        bRst = true;
                        break;
                    }
                    QThread::msleep(100);
                }
                qDebug() << outStd;
                if (bRst) {
                    break;
                }
                QThread::msleep(200);
            }
        }

        // m_pythonProcess.write("exit\n");
        if (!bRst) {
            QMessageBox::information(this, "提示", "配置失败");
            return;
        }

        //python stream_on_by_channel.py --channel=0

    //    m_pythonProcess.write(sCmd.toStdString().data());


        //python stream_on_by_channel.py --channel=0
    //    sCmd = "python stream_on_by_channel.py --channel=" +
    //           QString::number(2 * iGroup + 1) +
    //           "\n";
    //    m_pythonProcess.write(sCmd.toStdString().data());

        // int iCount = 600;
        // while (iCount-- > 0) {
        //     m_pythonProcess.waitForFinished(1000);
        //     QString sOut = m_pythonProcess.readAllStandardOutput();
        //     if(sOut.size() > 0)
        //         qInfo()<<sOut;
        //     sOut = m_pythonProcess.readAllStandardError();
        //     if(sOut.size() > 0)
        //         qInfo()<<sOut;
        //     QCoreApplication::m_pythonProcessEvents();
        // }
        // iCount = 200;
        // while(iCount-- > 0) {
        //     QThread::usleep(10 * 1000);
        //     QCoreApplication::processEvents();
        // }
        // sCmd = "python " + sAppPath + "/python/stream_on_by_channel.py --channel=" +
        //     QString::number(iChannel) + "\n";
        // system(sCmd.toStdString().data());
    } else if (typeId == TYPEARTOSYN) {
        // ArtosynNet* net = new ArtosynNet();
        // if (!net->netConnect("192.168.1.11", 6000)) {
        //     QMessageBox::information(this, "提示", "配置失败");
        // }
    }

    QMessageBox::information(this, "提示", "配置完成");
}

void TransConfig::on_connectEditChanged(QWidget *w)
{
    QLineEdit * cur = (QLineEdit *) w;
    int idxRow = getRowIdx(w, 1);
}

void TransConfig::removeItem(int row)
{
    QLayoutItem *item = ui->gridLayout->itemAtPosition(row, 5);
    if(item && !item->isEmpty() && item->widget()) {
        item->widget()->hide();
        ui->gridLayout->removeWidget(item->widget());

    }
    item = ui->gridLayout->itemAtPosition(row, 4);
    if(item && !item->isEmpty() && item->widget()) {
        item->widget()->hide();
        ui->gridLayout->removeWidget(item->widget());

    }
    item = ui->gridLayout->itemAtPosition(row, 3);
    if(item && !item->isEmpty() && item->widget()) {
        item->widget()->hide();
        ui->gridLayout->removeWidget(item->widget());

    }
    item = ui->gridLayout->itemAtPosition(row, 2);
    if(item && !item->isEmpty() && item->widget()) {
        item->widget()->hide();
        ui->gridLayout->removeWidget(item->widget());
    }
}

int TransConfig::getRowIdx(QWidget *w, int col)
{
    int row = ui->gridLayout->rowCount();
    int idxRow = 0;
    for(int i = 0; i < row; i++) {
        QLayoutItem *item = ui->gridLayout->itemAtPosition(i,col);
        if(!item) {
            continue;
        }
        QWidget * lItem = (QWidget*)item->widget();
        if(w == lItem) {
            idxRow = i;
            break;
        }
    }
    return idxRow;
}

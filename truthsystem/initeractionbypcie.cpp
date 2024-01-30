#include "initeractionbypcie.h"
#include <alg_sdk/service.h>
#include <QCoreApplication>
#include <QDebug>
#include <QThread>
#include <QElapsedTimer>
#include <QMessageBox>
CInteractionByPcie::CInteractionByPcie(){}


// ./pcie_sdk_demo_service -write_reg 0 4 0x66 0x140 0x19 0x1616
// board id
// channel
// addr
// reg addr
// value
// bytes
bool CInteractionByPcie::readReg(uint8_t board_id, uint8_t ch_id, uint16_t i2c_add, uint16_t reg_addr, uint16_t type, unsigned char check)
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
bool CInteractionByPcie::writeReg(uint8_t board_id, uint8_t ch_id, uint8_t i2c_add, uint16_t reg_addr, uint8_t value, uint16_t type)
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

void CInteractionByPcie::sendMsgByConsole(QString curText,int group,int channel)
{
    QString sAppPath = QCoreApplication::applicationDirPath();
    QString spythonPath = sAppPath + "/python";
    m_pythonProcess.start("bash");
    m_pythonProcess.waitForStarted();
    QString sCmd = "cd  "+spythonPath+"\n";
    qInfo()<<sCmd;
    m_pythonProcess.write(sCmd.toStdString().data());

    sCmd = "python3 set_sensor_from_json.py --json_file='" + sAppPath + "/config/" +
            curText + "' --channel_id=" + QString::number(group) + "\n";
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
        sCmd = "python3 " + sAppPath + "/python/stream_on_by_channel.py --channel=" + QString::number(channel) + "\n";
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
            while (timer.elapsed() < 5000) {
                QCoreApplication::processEvents();
                outStd += m_pythonProcess.readAll();
                // if (outStd.contains("result = 0")) {
                if(outStd.contains("ack code [0]")){
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
    if(curText.contains("ox03c_1_ch_slave")){//该设备需要触发信号才能出图
        sCmd = QString("python3 %1/python/set_trigger_mode.py --device=0 --mode=2\n").arg(sAppPath); 
        m_pythonProcess.write(sCmd.toStdString().data());
        QThread::msleep(200);        
        sCmd = QString("python3 %1/python/set_trigger_int.py --channel=%2 --mode=2 --freq=30 --delay_time=0 --valid_time=1\n").arg(sAppPath).arg(channel); 
        m_pythonProcess.write(sCmd.toStdString().data());
        QThread::msleep(200);
    }
    // m_pythonProcess.write("exit\n");
    if (!bRst) {
        QMessageBox::information(NULL, "提示", "配置失败");
        return;
    }
    QMessageBox::information(NULL, "提示", "配置完成");
}
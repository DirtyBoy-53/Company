#include "AlgSdkManager.h"
#include <QDebug>
#include <memory>
#include <QDateTime>
#include <QDebug>
#include <QCoreApplication>
#include <QElapsedTimer>
#include <QThread>
#include <algorithm>

#include "alg_sdk/alg_sdk.h"
#include "alg_sdk/service.h"
#include "alg_sdk/client.h"
#include "alg_common/basic_types.h"

#include "Window.h"
#include "YDefine.h"
#include "confile.h"
#include "ylog.h"
#include "YFunction.h"
void callback_image_data(void *p);

AlgSdkManager::AlgSdkManager()
{
    connect(&m_process, &QProcess::readyReadStandardOutput, this, [=](){
        QString msg = QString::fromLocal8Bit(m_process.readAllStandardOutput().data());
        qDebug()<< msg;
        // std::cout << m_process->readAllStandardOutput().data() << std::endl;
    });
}

bool AlgSdkManager::init()
{
    m_thread_init = new std::thread([](){
        int ret;
        char image_topic_names[ALG_SDK_MAX_CHANNEL][256]{0};
        for (auto i = 0; i < VIDEO_WINDOW_MAX; i++)
        {
            snprintf(image_topic_names[i], 256, "/image_data/stream/%02d", i);
            qInfo("subscribe %s",image_topic_names[i]);
            ret = alg_sdk_subscribe(image_topic_names[i], callback_image_data);
            if (ret < 0) {
                QString msg = QString::asprintf("Subscribe to topic %s Error!:%s",image_topic_names[i]);
                qWarning() << msg;
                // throw CAlgSdkException(msg.toStdString());
                return false;
            }
        }

        qDebug() << "start alg_sdk_init_client";
        if(alg_sdk_init_client()) {
            qWarning() << "Client init failed!";
            // throw CAlgSdkException("Client init failed!");
            return false;
        }
        alg_sdk_client_spin_on();
    });
    return true;
}
#include <pthread.h>
void AlgSdkManager::close()
{
    if(!m_closeFlag){
        alg_sdk_stop();
        alg_sdk_stop_client();
        pthread_cancel(m_thread_init->native_handle());
        m_thread_init->join();
        pthread_cancel(m_thread_open->native_handle());
        m_thread_open->join();
        m_closeFlag = true;
    }

}

Window *AlgSdkManager::window() const
{
    return m_window;
}

void AlgSdkManager::setWindow(Window *window)
{
    m_window = window;
}


bool AlgSdkManager::writeCameraConf(QString conf, int group, int channel)
{
    // QProcess    process;
    auto &process = m_process;
    static const QString pythonPath = QString("%1/pcie_conf/python").arg(g_exec_dir_path);
    static const QString pytonCmd = QString("cd %1\n").arg(pythonPath);
    const QString fileCmd = QString("python3 %1/set_sensor_from_json.py --json_file='%2/config/%3' --channel_id=%4\n")
                                .arg(pythonPath).arg(pythonPath).arg(conf).arg(group);
    const QString channelCmd = QString("python3 %1/stream_on_by_channel.py --channel=%2\n")
                                   .arg(pythonPath).arg(channel);
    const QString triggerModeCmd = QString("python3 %1/set_trigger_mode.py --device=0 --mode=2\n").arg(pythonPath);
    const QString triggerCmd = QString("python3 %1/set_trigger_int.py --channel=%2 --mode=2 --freq=30 --delay_time=0 --valid_time=1\n")
                                   .arg(pythonPath).arg(channel);

    QStringList cmdStrList,retStrList;
    cmdStrList.append(fileCmd);
    cmdStrList.append(channelCmd);
    retStrList.append(QString("result ="));
    retStrList.append(QString("ack code [0]"));

    process.start(QString("bash"));
    process.waitForStarted();
    process.write(pytonCmd.toStdString().c_str());
    std::cout << pytonCmd.toStdString();
    bool ret{false};
    QElapsedTimer timer;
    for(int i = 0;i < cmdStrList.size();++i){
        process.write(cmdStrList[i].toStdString().c_str());
        std::cout << "write:" << cmdStrList[i].toStdString();
        process.waitForBytesWritten(10000);

        timer.start();
        QByteArray outStr;
        while (timer.elapsed() < 20000) {
            QCoreApplication::processEvents();
            outStr += process.readAll();
            if (outStr.contains(retStrList[i].toStdString().c_str())) {
                ret = true;
                break;
            }
            QThread::msleep(100);
        }

        if(ret == false) {
            qWarning() << QString("%1 %2 %3 写配置错误").arg(conf).arg(group).arg(channel);
            break;
        }
    }

    if(conf.contains("ox03c_1_ch_slave")){//该设备需要触发信号才能出图
        process.write(triggerModeCmd.toStdString().c_str());
        QThread::msleep(200);
        process.write(triggerCmd.toStdString().c_str());
        QThread::msleep(200);
    }

    return ret;
}

bool AlgSdkManager::open()
{
    m_thread_open = new std::thread([](){
        int rc;
        char *appsrc[] = {"--publish"};
        qDebug() << "start alg_sdk_init_v2";
        rc = alg_sdk_init_v2(1, &appsrc[0]);
        qDebug() << "end alg_sdk_init_v2" << rc;
        if(rc < 0) {
            // throw CAlgSdkException("algSdk open Failed.");
            return false;
        }
        qDebug() << "start alg_sdk_spin_on";
        alg_sdk_spin_on();
    });
    m_closeFlag = false;
     return true;
}

FrameRecord m_lastInfo[VIDEO_WINDOW_MAX];
void writefile(char* frame, int len);
void callback_image_data(void *p)
{
    //解析通道号
    pcie_image_data_t* data = (pcie_image_data_t*)p;
    char* name = data->common_head.topic_name;

    QString sName = QByteArray::fromRawData(name, strlen(name));
    if(sName.isEmpty()){
     qWarning() << "相机回调中设备名称是空的";
     return;
    }
    int channelId = sName.right(2).toInt();
    // qInfo() << QString("数据接受 通道号:%1").arg(channelId);
    if(channelId < 0 || channelId >= VIDEO_WINDOW_MAX){
     qWarning() << QString("通道号异常:%1").arg(channelId);
     return;
    }

    //计算fps
    uint64_t tLast = m_lastInfo[channelId].tm;
    uint64_t tCur = QDateTime::currentMSecsSinceEpoch();
    if(tLast + 10000 <= tCur) {
     m_lastInfo[channelId].fps = m_lastInfo[channelId].count;
     m_lastInfo[channelId].count = 0;
     m_lastInfo[channelId].tm = tCur;
     // qInfo()<< "channel "<< channelId <<" fps "<<m_lastInfo[channelId].fps/10.0;
    }
    m_lastInfo[channelId].count++;
    //装载头部数据
    FrameInfo_S head;
    auto meta = data->image_info_meta;
    head.uFormat    = VIDEO_FORMAT_UYVY;   //y8 uyvy
    head.uTotalLen  = meta.img_size + FRAME_HEADSIZE_MAX;//大小
    head.width      = meta.width;
    head.height     = meta.height;
    head.uTimeTm    = meta.timestamp;
    head.ext[0]     = meta.frame_index;
    head.ext[1]     = m_lastInfo[channelId].fps/10;
    head.ext[2]     = meta.data_type;

    uint8_t *pData{nullptr};
    //参数行
    int offset{ 0 };
    if (head.ext[2] == PCIE_CAMERA_IR) {        // 红外相机
        head.uParamLen  = head.width * 2 * 2;
        head.height     = data->image_info_meta.height - 4;
        pData = (uint8_t*)data->payload;
        offset = head.width * (head.height + 2) * 2;//(w*h(图像数据)+w*2(锅盖数据:无意义))*2

        if (pData[offset] == 0x55 && pData[offset + 1] == 0xAA) {//参数行帧头
            head.uFormat = pData[offset + 17];//图像格式
        }
        if (head.uFormat == VIDEO_FORMAT_UYVY) {
            head.uYuvLen = head.width * head.height * 2;
        } else if (head.uFormat == VIDEO_FORMAT_Y16) {
            head.uY16Len = head.width * head.height * 2;
        }
    }
    else if(head.ext[2] == PCIE_CAMERA_LIGHT) { // 可见光相机
        head.uHeadFlag = 0xefefefef80808080;
        head.vlLen = head.width * head.height * 2;
        head.vlFormat = VIDEO_FORMAT_NV12;//1:yuv420  4:yuv422
        head.vlWidth = head.width;
        head.vlHeight = head.height;
    }
    // qDebug() << QString("type:%1,w:%2,h:%3,format:%4, fps:%5, uTotalLen:%6, imgSize:%7")
    //                 .arg(head.ext[2]).arg(head.width).arg(head.height).arg(head.uFormat).arg(head.ext[1]).arg(head.uTotalLen).arg(meta.img_size);

    //抽取数据
    XYZJFramePtr frame = std::make_shared<XYZJFrame>(head.uTotalLen-FRAME_HEADSIZE_MAX);
    frame->paramOffset = offset;
    memcpy(&frame->head, &head, sizeof(head));

    memcpy(frame->data, (char*)pData, head.uTotalLen-FRAME_HEADSIZE_MAX);

    //传入视频窗口
    AlgSdkManager::instance()->window()->sendFrame(frame, channelId);
}

ConsoleControl::ConsoleControl(QObject *obj)
    : QObject(obj)
    , m_process(new QProcess)
{
    // m_process = new QProcess;
    connect(m_process, &QProcess::readyReadStandardOutput, this, [=](){
        QString msg = QString::fromLocal8Bit(m_process->readAllStandardOutput().data());
        m_msg.append(msg);
        qDebug()<< msg;
        // std::cout << m_process->readAllStandardOutput().data() << std::endl;
    });
}

ConsoleControl::~ConsoleControl()
{
    m_process->kill();
    m_process->close();
    delete m_process;
    m_process = nullptr;
}
#include <sys/types.h>
bool ConsoleControl::writeCameraConf(QString conf, int group, int channel)
{
    qDebug() << "writeCameraConf threadId:" << gettid();
    QProcess *process = m_process;
    static const QString pythonPath = QString("%1/pcie_conf/python").arg(g_exec_dir_path);
    static const QString pytonCmd = QString("cd %1\n").arg(pythonPath);
    const QString fileCmd = QString("python3 %1/set_sensor_from_json.py --json_file='%2/config/%3' --channel_id=%4\n")
                                .arg(pythonPath).arg(pythonPath).arg(conf).arg(group);
    const QString channelCmd = QString("python3 %1/stream_on_by_channel.py --channel=%2\n")
                                   .arg(pythonPath).arg(channel);
    const QString triggerModeCmd = QString("python3 %1/set_trigger_mode.py --device=0 --mode=2\n").arg(pythonPath);
    const QString triggerCmd = QString("python3 %1/set_trigger_int.py --channel=%2 --mode=2 --freq=30 --delay_time=0 --valid_time=1\n")
                                   .arg(pythonPath).arg(channel);

    process->start(QString("bash"));
    process->waitForStarted();
    process->write(pytonCmd.toStdString().c_str());
    std::cout << pytonCmd.toStdString();
    bool ret{false};
    QElapsedTimer timer;

    process->write(fileCmd.toStdString().c_str());
    QCoreApplication::processEvents();
    // process->waitForBytesWritten(10000);
    timer.start();

    while (timer.elapsed() < 20000) {
        QCoreApplication::processEvents();
        if (m_msg.contains("result =")) {
            ret = true;
            break;
        }
        QThread::msleep(10);
    }
    if(ret == false) {
        qWarning() << QString("%1 %2 %3 写配置错误").arg(conf).arg(group).arg(channel);
        return false;
    }
    ret = false;

    for(auto i = 0;i < 4;i++){
        process->write(channelCmd.toStdString().c_str());
        QCoreApplication::processEvents();
        // process->waitForBytesWritten(1000);
        timer.start();

        while (timer.elapsed() < 2000) {
            QCoreApplication::processEvents();
            if (m_msg.contains("ack code [0]")) {
                ret = true;
                break;
            }
            QThread::msleep(10);
        }
        if(ret == true) break;
    }
    if(ret == false) {
        qWarning() << QString("%1 %2 %3 开启视频通道失败").arg(conf).arg(group).arg(channel);
        return false;
    }

    // qDebug() << m_msg;
    // &std::cout << m_msg.toStdString();
    YLog::Logger->info(m_msg.toStdString());

    if(conf.contains("ox03c_1_ch_slave")){//该设备需要触发信号才能出图
        process->write(triggerModeCmd.toStdString().c_str());
        QThread::msleep(200);
        process->write(triggerCmd.toStdString().c_str());
        QThread::msleep(200);
    }

    return ret;
}

#include <fstream>
void writefile(char* frame, int len){
    QString filepath = g_exec_dir_path + "/yuv-raw.bin";
    std::ofstream out;
    out.open(filepath.toStdString(), std::ios::out);
    out.write(frame, len);
}

bool IICControl::writeRegByIIC(uint8_t ch_id, uint16_t reg_addr, uint16_t value, uint16_t i2c_add, uint16_t type, uint8_t board_id)
{
    QString msg = QString::asprintf("[channel:%d], [reg_addr:0x%x], [value:%d]", ch_id, reg_addr, value);
    qInfo() << msg;
    static const char *topic_name = "/service/camera/write_reg";
    service_camera_write_reg_t write_s = {
        .ack_mode = 1,          .ch_id = 0,     .msg_type = 0x1608,
        .device_addr = 0x80,    .line_len = 2,
    };

    write_s.ch_id = board_id*8 + ch_id;
    write_s.device_addr = i2c_add;
    write_s.msg_type = type;

    write_s.payload[0] = reg_addr;
    write_s.payload[1] = value;
    
    int rc = alg_sdk_call_service(topic_name, &write_s, 10000);
    if (rc < 0) {
        qWarning( "Request Service : [%s] Error!\n", topic_name);
        return false;
    }
    QString str = QString::asprintf("[ack:%d], [channel:%d]", write_s.ack_code, write_s.channel);
    qInfo() << str;
    if(0 != write_s.ack_code) {
        qWarning() << "write reg failed!";
        return false;
    }
    return true;
}

bool IICControl::write(uint8_t ch_id, uint16_t reg_addr, uint16_t value, uint16_t type, uint16_t i2c_add, uint8_t board_id)
{
    QString msg = QString::asprintf("[channel:%d], [reg_addr:0x%x], [value:%d]", ch_id, reg_addr, value);
    qInfo() << msg;
    static const char *topic_name = "/service/camera/write_reg";
    service_camera_write_reg_t write_s = {
        .ack_mode = 1,          .ch_id = 0,     .msg_type = 0x1608,
        .device_addr = 0x80,    .line_len = 2,
    };

    write_s.ch_id = board_id*8 + ch_id;
    write_s.device_addr = i2c_add;
    write_s.msg_type = type;
    write_s.line_len = 2;

    write_s.payload[0] = reg_addr;

    write_s.payload[1] = value;

    for(auto i = 0;i < 2; i++){
        qDebug("send:%d",write_s.payload[i]);
    }
    // write_s.payload[1] = value;
    


    int rc = alg_sdk_call_service(topic_name, &write_s, 10000);
    if (rc < 0) {
        qWarning( "Request Service : [%s] Error!\n", topic_name);
        return false;
    }
    QString str = QString::asprintf("[ack:%d], [channel:%d]", write_s.ack_code, write_s.channel);
    qInfo() << str;
    if(0 != write_s.ack_code) {
        qWarning() << "write reg failed!";
        return false;
    }

    return true;
}

readReg_S IICControl::readRegByIIC(uint8_t ch_id,  uint16_t reg_addr, uint16_t size, uint16_t i2c_add, uint16_t type, uint8_t board_id)
{
    static const char *topic_name = "/service/camera/read_reg";
    QString Msg = QString::asprintf("[channel:%d], [reg_addr:0x%x], [size:%d]", ch_id, reg_addr, size);
    qInfo() << Msg;
    service_camera_read_reg_t read_s = {
        .ack_mode = 1,          .ch_id = 0,     .msg_type = 0x1608,
        .device_addr = 0x90,    .line_len = 1,
    };
    readReg_S rs;
    static const int singleReadSize{4};//单次读取字节数 0x1632表示一次读取4字节
    int rc{0};
    int readCnt = size*2/singleReadSize + (size*2%singleReadSize == 0 ? 0 : 1)  ;//读取的数据总是会增加一倍无效数据（从机增加） readCnt = 总大小÷单次读取字节数
    for(auto i = 0;i < readCnt; i++){
        read_s.ack_mode     = 1;                    read_s.line_len     = 1;
        read_s.ch_id        = board_id*8 + ch_id;   read_s.device_addr  = i2c_add;  
        read_s.msg_type     = type;                 read_s.payload[0]   = reg_addr;

        rc = alg_sdk_call_service(topic_name, &read_s, 10000);
        std::reverse(read_s.data, read_s.data + read_s.length_r);
        for(auto k = 0,cnt = 1; k < singleReadSize/2; ++k,cnt+=2){
            rs.value[i*singleReadSize/2 + k] = read_s.data[cnt];
        }
        for(auto j = 0;j < read_s.length_r;++j)
            qDebug() << "read_s recv:" << read_s.data[j];
        rs.len +=  read_s.length_r/2;
        QThread::msleep(5);
    }

    rs.channel  = read_s.channel;
    rs.ret      = read_s.ack_code == 0 ? true : false;

    if (rc < 0) {
        rs.ret = false;
        qWarning( "Request Service : [%s] Error!\n", topic_name);
        return rs;
    }
    qInfo() << QString::asprintf("[ack: %d], [channel: %d], [length: %d]", read_s.ack_code, read_s.channel, read_s.length_r);
    if(0 != read_s.ack_code) {
        rs.ret = false;
        qWarning() << "read reg failed!";
        return rs;
    }
    QString msg("Rs Rcv:");
    for(size_t i = 0;i < (rs.len > size ? size : rs.len);i++){
        msg.append(QString::asprintf("0x%x ",rs.value[i]));
    }
    yprint(msg, PRINT_GREEN);
    return rs;
}

bool AlgSdkManager::writeReg(uint8_t board_id, uint8_t ch_id, uint8_t i2c_add, uint16_t reg_addr, uint8_t value, uint16_t type)
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
    t1.ch_id = board_id*8 + ch_id;
    t1.device_addr = i2c_add;
    reg[0] = reg_addr;
    reg[1] = value;
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

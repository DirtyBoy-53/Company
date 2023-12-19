#include "mcuserialmanager.h"
#include <QThread>
#include <QDebug>
#include <QDate>

McuSerialManager::McuSerialManager()
{
    m_serial = new CSerialPort();
}

McuSerialManager::~McuSerialManager()
{

}

QString McuSerialManager::errString()
{
    return mErrString;
}

bool McuSerialManager::openSerial(const QString &mcuCom)
{
    SerialParam param;
    param.baudRate    = QSerialPort::Baud115200;
    param.dataBits    = QSerialPort::Data8;
    param.flowControl = QSerialPort::NoFlowControl;
    param.parity      = QSerialPort::NoParity;
    param.stopBits    = QSerialPort::OneStop;

    return m_serial->openSerial(mcuCom, param, true);
}

void McuSerialManager::closeSerial()
{
    m_serial->closeSerial();
    m_cache.clear();
    QThread::msleep(1000);
}

int McuSerialManager::version()
{
    return m_versionId;
}

bool McuSerialManager::pressed()
{
    SysWorkInfo infor;
    infor.bPressStatus = false;
    if (!getSysWorkInfo(infor)) {
        mErrString = "get failed";
        return false;
    }

    m_versionId = infor.nVersion;
//    qDebug() << "pressed is " << infor.bPressStatus;
    return infor.bPressStatus;
}

bool McuSerialManager::changeMcuWorkMode(const McuWorkMode mode)
{
    unsigned char cmd[11] = {0x55, 0xAA, 0x06, 0x00, 0x04, 0x00, 0x00, 0x00, 0x00, 0xF0, 0xF0};
    cmd[8] = (int)mode;
    getCRCCmd(cmd, 11);
    m_serial->sendCommandWait(cmd, 11);
    bool bret = checkRespAck();

    //QThread::sleep(1); //模式生效时间

    return bret;
}

bool McuSerialManager::setSysMode(SysWorkMode mode)
{
    unsigned char ucMode = 0x00;
    switch(mode) {
    case MODE_SN_DATE:
        ucMode = 0x01;
        break;
    case MODE_CAN_232:
        ucMode = 0x02;
        break;
    default: // MODE_USB_ADC
        ucMode = 0x00;
    }

    unsigned char cmd[11] = {0x55, 0xAA, 0x06, 0x00, 0x04, 0x00, 0x00, 0x00, 0x00, 0xF0, 0xF0};
    cmd[8] = ucMode;
    getCRCCmd(cmd, 11);
    m_serial->sendCommandWait(cmd, 11);
    bool ack = checkRespAck();
    //QThread::msleep(50);
    return ack;
}

void McuSerialManager::getCRCCmd(unsigned char *cmd, const unsigned int len)
{
    if(cmd == nullptr || len < 11){
        return;
    }

    char checkxor = cmd[3];
    for(int i = 4; i <= 8; i++){
        checkxor ^= cmd[i];
    }
    cmd[9] = checkxor;
}

int McuSerialManager::getMcuCrcCmd(unsigned char *cmd, const int len)
{
    if(cmd == nullptr){
        return 0;
    }

    char checkxor = cmd[0];
    for(int i = 1; i < len-1; i++){
        checkxor ^= cmd[i];
    }
    return checkxor;
}

bool McuSerialManager::cmpCRCCmd(unsigned char *cmd, const unsigned int len)
{
    if(cmd == nullptr || len != 9){
        return false;
    }

    char checkxor = cmd[2];
    for(int i = 3; i < len - 2; i++){
        checkxor ^= cmd[i];
    }
    return (cmd[7] == checkxor);
}

bool McuSerialManager::checkRespAck()
{
    QByteArray resp = m_serial->readSerialHold(1000, 6);
    if(resp.size() < 6) {
        return false;
    }

    if((uchar)resp.at(0) == 0x55 && (uchar)resp.at(1) == 0xaa && (uchar)resp.at(2) == 0x01 &&
            (uchar)resp.at(3) == 0x00 && (uchar)resp.at(4) == 0x01 && (uchar)resp.at(5) == 0xF0) {
        return true;
    } else {
        return false;
    }
}

bool McuSerialManager::getSysWorkInfo(SysWorkInfo &info)
{
    unsigned char cmd[11] = {0x55, 0xAA, 0x06, 0x00, 0x80, 0x00, 0x00, 0x00, 0x00, 0xF0, 0xF0};
    getCRCCmd(cmd, 11);
    unsigned char * ack = (unsigned char*)m_serial->sendWaitForResp(cmd, 11, 6).data();

    if(ack[0] == 0x55 && ack[1] == 0xaa && ack[2] == 0x01 &&
       ack[3] == 0x00 && ack[4] == 0x01 && ack[5] == 0xF0) {
        QByteArray recv = m_serial->read(3);
        unsigned char* headData = (unsigned char*)recv.data();
        if(headData[0] != 0x55 || headData[1] != 0xAA) {
            mErrString = "Returned Header Fail";
            return false;
        }

        int readLen = headData[2];
        recv.append(m_serial->read(readLen+2));
        unsigned char* recvData = (unsigned char*)recv.data();
        info.bPressStatus = (recvData[4] == 0x01);
        info.nVersion = recvData[5];
        info.nServer2 = recvData[6];
        return true;
    }

    return false;
}

bool McuSerialManager::getMcuWorkInfo(McuWorkInfo& info)
{
    setSysMode(MODE_USB_ADC);
    unsigned char cmd[11] = {0x55, 0xAA, 0x06, 0x01, 0x80, 0x00, 0x00, 0x00, 0x00, 0xF0, 0xF0};
    getCRCCmd(cmd, 11);
//    unsigned char* ack = (unsigned char*)m_serial->sendWaitForResp(cmd, 11, 6).data();
    m_serial->sendCommandWait((unsigned char*)cmd, 11);
    QByteArray read = m_serial->readSerialHold(2000, 53);
    if(read.size() <= 0) {
        return false;
    }
    unsigned char* ack = (unsigned char*)read.data();
    qInfo()<<"read ack "<<QString(read.toHex());
    if(ack[0] == 0x55 && ack[1] == 0xaa && ack[2] == 0x01 &&
       ack[3] == 0x00 && ack[4] == 0x01 && ack[5] == 0xF0) {
        unsigned char* resp = (unsigned char*)ack + 6;
        if(resp[0] != 0x55 || resp[1] != 0xAA) {
            return false;
        }
        info.volOut = float(resp[4] << 8 | resp[5])/100;
        info.volAdc1_inp2 = float(resp[6] << 8 | resp[7])/100;
        info.volAdc1_inp5 = float(resp[8] << 8 | resp[9])/100;
        info.volAdc1_inp6 = float(resp[10] << 8 | resp[11])/100;
        info.volAdc1_inp9 = float(resp[12] << 8 | resp[13])/100;
        info.volAdc1_inp10 = float(resp[14] << 8 | resp[15])/100;
        info.volAdc1_inp16 = float(resp[16] << 8 | resp[17])/100;
        info.volAdc2_inp2 = float(resp[18] << 8 | resp[19])/100;
        info.volAdc2_inp6 = float(resp[20] << 8 | resp[21])/100;
        info.volAdc3_inp0 = float(resp[22] << 8 | resp[23])/100;
        info.volAdc3_inp1 = float(resp[24] << 8 | resp[25])/100;
        info.volAdc3_inp4 = float(resp[26] << 8 | resp[27])/100;
        info.volAdc3_inp5 = float(resp[28] << 8 | resp[29])/100;
        info.volAdc3_inp7 = float(resp[30] << 8 | resp[31])/100;
        info.volAdc3_inp9 = float(resp[32] << 8 | resp[33])/100;
        info.volAdc3_inp13 = float(resp[34] << 8 | resp[35])/100;
        info.volAdc3_inp15 = float(resp[36] << 8 | resp[37])/100;
        info.volAdc3_inp16 = float(resp[38] << 8 | resp[39])/100;

        int year = 2000 + resp[41];
        int month = resp[42];
        int day = resp[43];
        int v1 = resp[44];
        int v2 = resp[45];
        int v3 = resp[46];
        info.version = "V" + QString::number(v1) + "." + QString::number(v2) + "." +
                QString::number(v3) + "." + QString::number(year)+QString("%1%2").arg(month,2,10,QLatin1Char('0')).arg(day,2,10,QLatin1Char('0'));

        qDebug() << QString::number(info.volOut) << QString::number(info.volAdc1_inp2) << QString::number(info.volAdc1_inp5) << QString::number(info.volAdc1_inp6) << QString::number(info.volAdc1_inp9)
                 << QString::number(info.volAdc1_inp10) << QString::number(info.volAdc1_inp16) << QString::number(info.volAdc2_inp2) << QString::number(info.volAdc2_inp6)
                 << QString::number(info.volAdc3_inp0) << QString::number(info.volAdc3_inp1) << QString::number(info.volAdc3_inp4) << QString::number(info.volAdc3_inp5)
                 << QString::number(info.volAdc3_inp7) << QString::number(info.volAdc3_inp9) << QString::number(info.volAdc3_inp13) << QString::number(info.volAdc3_inp15)
                 << QString::number(info.volAdc3_inp16) << info.version;
        return true;
    } else {
        return false;
    }
}

bool McuSerialManager::getArmWorkInfo(ArmWorkInfo &info)
{
    //changeMcuWorkMode(MCU_WORK_MODE_CHECK);
    setSysMode(MODE_CAN_232);
    unsigned char cmd[11] = {0x55, 0xAA, 0x06, 0x02, 0x80, 0x00, 0x00, 0x00, 0x00, 0xF0, 0xF0};
    getCRCCmd(cmd, 11);
    // unsigned char* ack = (unsigned char*)m_serial->sendWaitForResp(cmd, 11, 6).data();

    m_serial->sendCommandWait((unsigned char*)cmd, 11);
    QByteArray read = m_serial->readSerialHold(2000, 32);
    if(read.size() <= 0) {
        return false;
    }
    unsigned char* ack = (unsigned char*)read.data();
    qInfo()<<"read ack "<<QString(read.toHex());
    if(ack[0] == 0x55 && ack[1] == 0xaa && ack[2] == 0x01 &&
       ack[3] == 0x00 && ack[4] == 0x01 && ack[5] == 0xF0) {
        // QByteArray recv = m_serial->read(3);
        unsigned char* recvData = (unsigned char*)ack + 6;
        if(recvData[0] != 0x55 || recvData[1] != 0xAA) {
            return false;
        }

//        int readLen = headData[2];
//        recv.append(m_serial->read(readLen+2));

//        QByteArray log((const char*)recv, recv.length());
//        qDebug() << "GET RESP data" << QString(log.toHex()) << readLen+2;

        // unsigned char* recvData = (unsigned char*)recv.data();
        info.elecValue = float(recvData[4] << 8 | recvData[5]) / 100 - 0.03; // todo 要改板，临时减0.03
        info.canWorkFlag = recvData[6];
        info.canWorkEnable = recvData[7];
        info.mcuAuthorCompleted = recvData[15];
        info.mcuAuthorValid = recvData[16];
        info.mcuVersion = QString("V%1.%2").arg(recvData[8]).arg(recvData[9]);
        info.armVersion = QString("V%1.%2.%3").arg(recvData[12]).arg(recvData[13]).arg(recvData[14]);
        info.rs232CheckFlag = recvData[17];
        info.rs232CheckResult = recvData[18];
        info.snWriteResult = recvData[19];
        info.oemFlag = recvData[21];
        int oem1 = (recvData[22] >> 4) & 0x0F;
        int oem2 = recvData[22] & 0x0F;
        QString str = QString("%1").arg(recvData[23], 2, 16, QLatin1Char('0'));
        int datt = str.toInt();
        info.oemHwVersion = QString("V%1.%2.%3").arg(oem1).arg(oem2).arg(datt, 2, 10, QLatin1Char('0'));
        int oem3 = (recvData[25] >> 4) & 0x0F;
        int oem4 = recvData[25] & 0x0F;
        QString strX = QString("%1").arg(recvData[26], 2, 16, QLatin1Char('0'));
        int dattX = strX.toInt();
        info.oemSwVersion = QString("V%1.%2.%3").arg(oem3).arg(oem4).arg(dattX, 2, 10, QLatin1Char('0'));
        qDebug() << QString::number(info.elecValue)
                 << QString::number(info.canWorkFlag)
                 << QString::number(info.canWorkEnable)
                 << QString::number(info.rs232CheckFlag)
                 << QString::number(info.mcuAuthorCompleted)
                 << QString::number(info.mcuAuthorValid)
                 << QString::number(info.rs232CheckResult)
                 << QString::number(info.snWriteResult)
                 << QString::number(info.oemFlag)
                 << info.mcuVersion << info.armVersion << info.oemHwVersion << info.oemSwVersion;
        return true;
    } else {
        return false;
    }
}

bool McuSerialManager::getArmWorkInfoB(ArmWorkInfo &info)
{
    unsigned char cmd[11] = {0x55, 0xAA, 0x06, 0x02, 0x80, 0x00, 0x00, 0x00, 0x00, 0xF0, 0xF0};
    getCRCCmd(cmd, 11);
    unsigned char* ack = (unsigned char*)m_serial->sendWaitForResp(cmd, 11, 6).data();
    if(ack[0] == 0x55 && ack[1] == 0xaa && ack[2] == 0x01 &&
       ack[3] == 0x00 && ack[4] == 0x01 && ack[5] == 0xF0) {
        QByteArray recv = m_serial->readSerialHold(1000, 21);
        qInfo()<<"recv "<<QString(recv.toHex());
        unsigned char* headData = (unsigned char*)recv.data();
        if(headData[0] != 0x55 || headData[1] != 0xAA) {
            return false;
        }

//        int readLen = headData[2];
//        recv.append(m_serial->read(readLen+2));
        unsigned char* recvData = (unsigned char*)recv.data();
        info.elecValue = float(recvData[4] << 8 | recvData[5]) / 100 - 0.03; // todo 要改板，临时减0.03
        info.canWorkFlag = recvData[6];
        info.canWorkEnable = recvData[7];
        info.mcuAuthorCompleted = recvData[15];
        info.mcuAuthorValid = recvData[16];
        info.mcuVersion = QString("V%1.%2").arg(recvData[8]).arg(recvData[9]);
        info.armVersion = QString("V%1.%2.%3").arg(recvData[12]).arg(recvData[13]).arg(recvData[14]);
        info.rs232CheckFlag = recvData[17];
        info.rs232CheckResult = recvData[18];
        info.snWriteResult = recvData[19];
        info.dateWriteResult = recvData[20];

        qDebug() << QString::number(info.elecValue) << QString::number(info.canWorkFlag)
                 << QString::number(info.canWorkEnable)  << QString::number(info.rs232CheckFlag) << QString::number(info.mcuAuthorCompleted) << QString::number(info.mcuAuthorValid)
                 << QString::number(info.rs232CheckResult) << info.mcuVersion << info.armVersion << QString::number(info.snWriteResult);
        return true;
    } else {
        return false;
    }
}

bool McuSerialManager::getSensorWorkInfo(SensorWorkInfo &info)
{
    unsigned char cmd[11] = {0x55, 0xAA, 0x06, 0x03, 0x80, 0x00, 0x00, 0x00, 0x00, 0xF0, 0xF0};
    getCRCCmd(cmd, 11);
    unsigned char* ack = (unsigned char*)m_serial->sendWaitForResp(cmd, 11, 6).data();
    if(ack[0] == 0x55 && ack[1] == 0xaa && ack[2] == 0x01 &&
       ack[3] == 0x00 && ack[4] == 0x01 && ack[5] == 0xF0) {
        QByteArray recv = m_serial->read(3);
        unsigned char* headData = (unsigned char*)recv.data();
        if(headData[0] != 0x55 || headData[1] != 0xAA) {
            return false;
        }

        int readLen = headData[2];
        recv.append(m_serial->read(readLen+2));

        unsigned char* recvData = (unsigned char*)recv.data();
        info.elecValue = float(recvData[4] << 8 | recvData[5]) / 100;
        int year = 2000 + recvData[7];
        int month = recvData[8];
        int day = recvData[9];
        int v1 = recvData[10];
        int v2 = recvData[11];
        int v3 = recvData[12];
        info.version = "V" + QString::number(v1) + "." + QString::number(v2) + "." +
                QString::number(v3) + "." + QString::number(year)+QString("%1%2").arg(month,2,10,QLatin1Char('0')).arg(day,2,10,QLatin1Char('0'));

        qDebug() << QString::number(info.elecValue) << info.version;
        return true;
    } else {
        return false;
    }
}

float McuSerialManager::getSensorVoltage()
{
    McuWorkInfo info;
    if (!getMcuWorkInfo(info)) {
        return -1;
    }

    return info.volAdc2_inp6;
}

float McuSerialManager::getArmVoltage()
{
    McuWorkInfo info;
    if (!getMcuWorkInfo(info)) {
        return -1;
    }

    return info.volAdc2_inp2;
}

float McuSerialManager::getSensorCurrent()
{
    SensorWorkInfo info;
    if (!getSensorWorkInfo(info)) {
        return -1;
    }

    return info.elecValue;
}

float McuSerialManager::getArmCurrent()
{
    ArmWorkInfo info;
    if (!getArmWorkInfo(info)) {
        return -1;
    }

    return info.elecValue;
}



bool McuSerialManager::controlGreenLight(const CommonSwitch mode)
{
    unsigned char cmd[11] = {0x55, 0xAA, 0x06, 0x00, 0x1, 0x00, 0x00, 0x00, 0x00, 0xF0, 0xF0};
    if(mode == COMMON_SWITCH_OFF) {
        cmd[8] = 0;
    } else {
        cmd[8] = 1;
    }
    getCRCCmd(cmd, 11);
    m_serial->sendCommandWait(cmd, 11);
    return checkRespAck();
}

bool McuSerialManager::controlRedLight(const CommonSwitch mode)
{
    unsigned char cmd[11] = {0x55, 0xAA, 0x06, 0x00, 0x2, 0x00, 0x00, 0x00, 0x00, 0xF0, 0xF0};
    if(mode == COMMON_SWITCH_OFF) {
        cmd[8] = 0;
    } else {
        cmd[8] = 1;
    }
    getCRCCmd(cmd, 11);
    m_serial->sendCommandWait(cmd, 11);
    return checkRespAck();
}

bool McuSerialManager::controlBuzzer(const CommonSwitch mode)
{
    unsigned char cmd[11] = {0x55, 0xAA, 0x06, 0x00, 0x3, 0x00, 0x00, 0x00, 0x00, 0xF0, 0xF0};
    if(mode == COMMON_SWITCH_OFF) {
        cmd[8] = 0;
    } else {
        cmd[8] = 1;
    }
    getCRCCmd(cmd, 11);
    m_serial->sendCommandWait(cmd, 11);
    return checkRespAck();
}

bool McuSerialManager::controlSysPower(const CommonSwitch mode)
{
    unsigned char cmd[11] = {0x55, 0xAA, 0x06, 0x01, 0x1, 0x00, 0x00, 0x00, 0x00, 0xF0, 0xF0};
    if(mode == COMMON_SWITCH_OFF) {
        cmd[8] = 0;
    } else {
        cmd[8] = 1;
    }
    getCRCCmd(cmd, 11);
    m_serial->sendCommandWait(cmd, 11);
    return checkRespAck();
}

bool McuSerialManager::controlArmPower(const int voltage, bool checkRst, bool step)
{
    setSysMode(MODE_USB_ADC);
    unsigned char cmd[11] = {0x55, 0xAA, 0x06, 0x02, 0x1, 0x00, 0x00, 0x00, 0x00, 0xF0, 0xF0};
    if (!step) {
        cmd[4] = 0x02;
    }
    cmd[8] = voltage;
    getCRCCmd(cmd, 11);
    m_serial->sendCommandWait(cmd, 11);
    bool brst = false;
    if (checkRst) {
        brst = checkRespAck();
    }
    m_currentArmPower = voltage;
    return brst;
}

bool McuSerialManager::controlArmPowerStepUp(const int voltage)
{
    int diff = voltage - m_currentArmPower;
    qDebug() << "diff is " << diff;
    int way = diff >= 0 ? 1 : -1;
    if (fabs(diff) <= 1) {
        controlArmPower(voltage);
    } else {
        // 丝滑渐进供电，不产生抖动，本来需要MCU去做的事情。
        for (int foo = (way * diff); foo >= 0 ; foo--) {
            controlArmPower(voltage - way * foo, true);
            //QThread::msleep(500);
            qDebug() << "power up " << voltage - way * foo;
        }
    }

    return true;
}

bool McuSerialManager::controlSensorPower(const int voltage, bool checkRst, bool step)
{
    setSysMode(MODE_USB_ADC);
    unsigned char cmd[11] = {0x55, 0xAA, 0x06, 0x03, 0x01, 0x00, 0x00, 0x00, 0x00, 0xF0, 0xF0};
    if (!step) {
        cmd[4] = 0x02;
    }
    cmd[8] = voltage;
    getCRCCmd(cmd, 11);
    m_serial->sendCommandWait(cmd, 11);
    if (checkRst) {
        return checkRespAck();
    }

    return true;
}

bool McuSerialManager::writeSn(const QString sn)
{
    setSysMode(MODE_SN_DATE);
    unsigned char cmd[11] = {0x55, 0xAA, 0x06, 0x02, 0xC0, 0x01, 0x00, 0x00, 0x00, 0xC3, 0xF0};
    m_serial->sendCommandWait(cmd, 11);
    if (!checkRespAck()) {
        qDebug() << "写SN前指令失败";
        return false;
    }
    QThread::msleep(1000);

    QByteArray snData = QByteArray::fromRawData(sn.toStdString().data(), sn.size());
    unsigned int len = snData.size() + 7;
    unsigned char send[64];
    send[0] = 0;
    send[1] = snData.size() + 4; //
    for(int i=0; i<snData.size(); i++) {
        send[i+2] = snData.at(i);
    }
    bool flag;
    send[len - 5] = QString::number(QDate::currentDate().year()/100).toInt(&flag, 16);
    send[len - 4] = QString::number(QDate::currentDate().year()).toInt(&flag, 16);
    send[len - 3] = QString::number(QDate::currentDate().month()).toInt(&flag, 16);
    send[len - 2] = QString::number(QDate::currentDate().day()).toInt(&flag, 16);
    send[len - 1] = getMcuCrcCmd(send, len);
    QByteArray log((const char*)send, len);
    qDebug() << "send SN data" << QString(log.toHex()) << len;
    int iRet = m_serial->sendCommandWait(send, len);
	QThread::msleep(1000);
    return iRet == 0? true:false;
}

bool McuSerialManager::readSn(QString &sn)
{
    unsigned char cmd[11] = {0x55, 0xAA, 0x06, 0x02, 0xE0, 0x01, 0x00, 0x00, 0x00, 0xE3, 0xF0};
    getCRCCmd(cmd, 11);

    for (int foo = 0; foo < 3; foo++) {
        QByteArray baOut = m_serial->sendWaitForResp(cmd, 11, 6);
        unsigned char* ack = (unsigned char*)baOut.data();
        if(ack[0] == 0x55 && ack[1] == 0xaa && ack[2] == 0x01 &&
           ack[3] == 0x00 && ack[4] == 0x01 && ack[5] == 0xF0) {
            QByteArray recv = m_serial->readSerialHold(1000, 6);
            if(recv.size() <= 0) {
                continue;
            }
            qInfo()<<"read "<<QString(recv.toHex());
            unsigned char* resp = (unsigned char*)recv.data();
            int snLen = resp[0] - 5;
            for(int i=1; i<=snLen; i++) {
                sn.append(resp[i]);
            }
            qDebug() << "-------------read sn is " << sn;
            return true;
        }
    }

    return false;

}

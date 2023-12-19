#include "fpchandler.h"
#include <QThread>

FpcHandler::FpcHandler()
{
    m_plcWrite = new CSyncSocket;
    m_plcRead = new CSyncSocket;
    m_camera = new CSyncSocket;
}

FpcHandler::~FpcHandler()
{

}

bool FpcHandler::initPlcWrite(QString ip, int port)
{
    m_plcWrite->connectToHost(ip, port);
    return m_plcWrite->waitForConnected();
}

bool FpcHandler::initPlcRead(QString ip, int port)
{
    m_plcRead->connectToHost(ip, port);
    return m_plcRead->waitForConnected();
}

bool FpcHandler::initCamera(QString ip, int port)
{
    m_camera->connectToHost(ip, port);
    return m_camera->waitForConnected();
}

bool FpcHandler::deInit()
{
    m_plcRead->disconnectFromHost();
    m_plcWrite->disconnectFromHost();
    m_camera->disconnectFromHost();
    return true;
}

bool FpcHandler::plcReadyOk()
{
    char cmd[12] = {0x0, 0x0, 0x0, 0x0, 0x0, 0x06, 0x01, 0x05, 0x24, 0x89, 0xFF, 0x00};
    m_plcWrite->write(cmd, 12);
    m_plcWrite->waitForBytesWritten();
    m_plcWrite->waitForReadyRead();
    QByteArray recv = m_plcWrite->readAll();
    qDebug() << "plcReadyOk " << recv;
    QThread::msleep(200);
    return true;
}

int FpcHandler::plcWaitStart()
{
    m_plcRead->waitForReadyRead(60*1000);
    QByteArray recv = m_plcRead->read(8);
    if(recv.isEmpty()) {
        return -1;
    }
    qDebug() << "plcWaitStart " << recv;
    return recv.at(4) == 0x10 ? 1 : 0;
}

bool FpcHandler::plcMoveTo(PlcMoveState state)
{
    char cmd[12] = {0x0, 0x0, 0x0, 0x0, 0x0, 0x06, 0x01, 0x05, 0x24, 0x88, 0xFF, 0x00};
    if(state == PLC_MOVE_SHUTTER_CHECK) {
        m_plcWrite->write(cmd, 12);
    } else if(state == PLC_MOVE_FPC_CHECK) {
        cmd[9] = 0x8A;
        m_plcWrite->write(cmd, 12);
    }

    m_plcWrite->waitForBytesWritten();
    m_plcWrite->waitForReadyRead();
    QByteArray recv = m_plcWrite->readAll();
    qDebug() << "plcMoveTo " << recv;
    QThread::msleep(200);
    return plcCheckError(0x12);
}

bool FpcHandler::plcCheck()
{
    char cmd[12] = {0x0, 0x0, 0x0, 0x0, 0x0, 0x06, 0x01, 0x05, 0x24, 0x8B, 0xFF, 0x00};
    m_plcWrite->write(cmd, 12);
    m_plcWrite->waitForBytesWritten();
    m_plcWrite->waitForReadyRead();
    QByteArray recv = m_plcWrite->readAll();
    qDebug() << "plcCheck " << recv;
    QThread::msleep(200);
    return plcCheckError(0x14);
}

bool FpcHandler::plcReset()
{
    char cmd[12] = {0x0, 0x0, 0x0, 0x0, 0x0, 0x06, 0x01, 0x05, 0x20, 0x5B, 0xFF, 0x00};
    m_plcWrite->write(cmd, 12);
    m_plcWrite->waitForBytesWritten();
    m_plcWrite->waitForReadyRead();
    m_plcWrite->readAll();
    QThread::msleep(200);
    return plcCheckError(0x13);
}

bool FpcHandler::plcAutoMode()
{
    char cmd[12] = {0x0, 0x0, 0x0, 0x0, 0x0, 0x06, 0x01, 0x05, 0x22, 0x14, 0xFF, 0x00};
    m_plcWrite->write(cmd, 12);
    m_plcWrite->waitForBytesWritten();
    m_plcWrite->waitForReadyRead();
    m_plcWrite->readAll();
    QThread::msleep(200);
    return 0;
}

bool FpcHandler::plcSetResult(const bool result)
{
    char cmd[12] = {0x0, 0x0, 0x0, 0x0, 0x0, 0x06, 0x01, 0x05, 0x24, 0x8C, 0xFF, 0x00};
    cmd[9] = result ? 0x8C : 0x90;
    m_plcWrite->write(cmd, 12);
    m_plcWrite->waitForBytesWritten();
    m_plcWrite->waitForReadyRead();
    m_plcWrite->readAll();
    QThread::msleep(200);
    return 0;
}

bool FpcHandler::plcWaitOrigin()
{
    QThread::msleep(200);
    m_plcRead->waitForReadyRead(600*1000);
    QByteArray recv = m_plcRead->read(8);
    qDebug() << "read recv " << recv;
    return recv.at(4) == 0x16;
}

bool FpcHandler::plcClearRead()
{
    m_plcRead->waitForReadyRead(1000);
    QByteArray recv = m_plcRead->readAll();
    qDebug() << "plc clear " << recv;
    return true;
}

FpcCheckResult FpcHandler::cameraCheck()
{
    m_camera->write("a\r\n");
    m_camera->waitForBytesWritten(1000);
    m_camera->waitForReadyRead(1000);
    QByteArray data = m_camera->readAll();
    qDebug() << "cameraCheck recv " << QString(data).simplified();

    FpcCheckResult fpcResult;
    QStringList list = QString(data).simplified().split("/");
    if(list.size() >= 3) {
        fpcResult.result = list[0] == "OK";
        QStringList mmList = list[1].split(":");
        QStringList jdcList = list[2].split(":");
        fpcResult.mm = mmList[1].toFloat();
        fpcResult.jdc = jdcList[1].toFloat();
    }
    qDebug() << "fpc result " << fpcResult.result << fpcResult.mm << fpcResult.jdc;
    QThread::msleep(200);
    return fpcResult;
}

bool FpcHandler::plcCheckError(int checkByte)
{
//    int retryTimes = 20;
//    while(retryTimes && m_plcRead->bytesAvailable()<8) {
//        m_plcRead->waitForReadyRead();
//        retryTimes--;
//        qDebug() << "plcCheckError " << retryTimes << m_plcRead->bytesAvailable();
//        if(m_plcRead->bytesAvailable()) break;
//    }
    m_plcRead->waitForReadyRead();
    QByteArray recv = m_plcRead->read(8);
    qDebug() << "read recv " << recv;
    return recv.at(4) == checkByte;
}

#include "civ3socket.h"
#include <QThread>
#include <QElapsedTimer>
#include <QCoreApplication>

CIv3Socket::CIv3Socket(QObject *parent) : QObject(parent)
{
    m_iv3SocketWindow = new QTcpSocket;
    m_iv3Socket = new QTcpSocket;
//    QThread* trd = new QThread;
//    moveToThread(trd);
//    trd->start();
}

CIv3Socket::~CIv3Socket()
{
    m_iv3Socket->disconnectFromHost();
    m_iv3SocketWindow->disconnectFromHost();
}

bool CIv3Socket::init(QString ip)
{
    m_iv3SocketWindow->connectToHost(ip, 63000);
    m_iv3Socket->connectToHost(ip, 8500);
    qDebug() << ">>>>>>Start Init CIv3Socket";
    QElapsedTimer timer;
    timer.start();
    while (timer.elapsed() < 10000) {
        QCoreApplication::processEvents();
        if (m_iv3Socket->state() == QTcpSocket::ConnectedState &&
                m_iv3SocketWindow->state() == QTcpSocket::ConnectedState) {
            return true;
        }
    }

    return false;
}

bool CIv3Socket::setProjectCode(QString code)
{
    //使用T2指令，触发检测并返回结果（TODO：结果数据格式暂时未知）
    QString _value{""};
    if(m_iv3Socket) {
        bool bOk = false;
        QString cmd = "PW," + code + "\r";
        qDebug() << "执行指令：" << cmd;
        for(auto i = 0;i < 5;i++){
            m_iv3Socket->write(cmd.toLocal8Bit().data(), cmd.size());
            QThread::msleep(2000);
            if (m_iv3Socket->waitForReadyRead(3000)) {
                QByteArray ba = m_iv3Socket->readAll();
                qDebug() << "读取指令结果：" << ba;
                _value = QString::fromLocal8Bit(ba.toStdString().c_str());
                qDebug() << "读取指令结果A：" << _value;
                if(_value.contains("PW") && !_value.contains("ER")) {
                    bOk = true;
                    break;
                }
            }
            QThread::msleep(1500);
        }

        cmd = "PR\r";
        qDebug() << "执行指令：" << cmd;
        for(auto i = 0;(i < 5) && bOk;i++){
            m_iv3Socket->write(cmd.toLocal8Bit().data(), cmd.size());
            QThread::msleep(500);
            if (m_iv3Socket->waitForReadyRead(3000)) {
                QByteArray ba = m_iv3Socket->readAll();
                qDebug() << "读取指令结果：" <<  ba;
                _value = QString::fromLocal8Bit(ba.toStdString().c_str());
                qDebug() << "读取指令结果A：" <<  _value;
                if(_value.contains(code)) {
                    return true;
                }
            }
        }

    }

    return false;
}

QString CIv3Socket::productCode()
{
    QString ret;

    //使用T2指令，触发检测并返回结果（TODO：结果数据格式暂时未知）
    QString _value{""};
    QStringList _valueList;
    if(m_iv3Socket){
        for(auto i = 0;i < 5;i++){
            m_iv3Socket->write("T2\r", 3);
            if (m_iv3Socket->waitForReadyRead(3000)) {
                QByteArray ba = m_iv3Socket->readAll();
                qDebug() << ba;
                _value = QString::fromLocal8Bit(ba.toStdString().c_str());
                qDebug() << _value;
                if(_value.contains("RT")) break;
            }
        }
    }

    if(_value.isEmpty())             //指令，次数，结果/编号，
        return "ERR";

    _valueList = _value.split(','); //RT,00013,OK,00,OK,0000094,01,OK,0000100

    //返回命令,次数，结果，
    if (_valueList.size() >= 7){ //TODO：收到结果后输出NG或者OK
        if (_valueList.at(2) == "OK" &&
                _valueList.at(4) == "OK" &&
                _valueList.at(7) == "OK")
        {
            return "OK";
        }

        /* 旧的识别方法
        QString _result = _valueList.at(2);
        if (_result == "00") { // XD01A
            ret = "XD01A";
        } else if (_result == "02"){ //XD03A
            ret = "XD03A";
        } else if (_result == "01"){ //XD01B
            ret = "XD01B";
        } else {
            ret = "NG";
        }
        */
    }else{
        ret = "NG";
    }

    return ret;
}

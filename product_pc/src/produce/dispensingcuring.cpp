#include "dispensingcuring.h"
#include <QThread>
#include <CPingTest.h>


DispensingCuring::DispensingCuring()
{
    connect(pMachine(), &QStateMachine::stopped,
            this, &DispensingCuring::slotStartWorkBtnClicked);

    m_StationName = ConfigInfo::getInstance()->cacheInfo().sCodeId;
    qDebug() << "Current Station Name: " << m_StationName;
    mMesObj.setUser(MesClient::instance()->sUser());
    int step{0};
    add(step++, "init", std::bind(&DispensingCuring::init, this));
    add(step++, "Judge", std::bind(&DispensingCuring::jugdeCustomLifeTime, this, true));
    add(step++, "doWork", std::bind(&DispensingCuring::doWork, this));
    add(step++, "finish", std::bind(&DispensingCuring::finish, this));

}

void DispensingCuring::init()
{
    if(!mShutterSocket.init()){
        m_errMsg = "网络连接失败";
        m_errorCode = -1;
    }
    m_sn = msgBox("获取产品SN");
    emit logAddNormalLog("产品SN:" + m_sn);
    setCurrentBarcode(m_sn); // 显示到界面

    if (isOnlineMode()) {
        QString out;
        logNormal("入站:" + m_sn);
        if (!mMesObj.enterProduce(m_sn, out)) {
            m_errMsg = "入站失败: " + out ;
            emit logAddFailLog("入站失败:" + out);
            mBoolMesIgnore = true;
            m_errorCode = -1;
            return;
        }
        logNormal("入站:" + m_sn + " ok");
        mBoolMesIgnore = false;

    } else {
        logWarming("当前为离线模式,不进行入站操作");
    }
}

void DispensingCuring::doWork()
{
    logNormal("放入产品后，请手动关闭左右门");
    QThread::msleep(500);
    while(!mShutterSocket.getCurStatus(door_is_close)){//等待关门完成
        QCoreApplication::processEvents();
        QThread::msleep(50);
    }
    logNormal("点胶中");
    if(!mShutterSocket.startDispensing()){//开始点胶
        m_errorCode = -2;
        m_errMsg = "发送点胶信号失败";
        addLog(m_errMsg, false);
        return;
    }
    logNormal("等待点胶完成");
    while(!mShutterSocket.getCurStatus(dispensing_is_over)){//等待点胶完成
        QCoreApplication::processEvents();
        QThread::msleep(50);
    }
    if(!mShutterSocket.startCuring()){//开始固化
        m_errorCode = -2;
        m_errMsg = "发送固化信号失败";
        addLog(m_errMsg, false);
        return;
    }
    logNormal("等待固化完成");
    while(!mShutterSocket.getCurStatus(curing_is_over)){//等待固化完成
        QCoreApplication::processEvents();
        QThread::msleep(50);
    }
    logNormal("等待开门信号");
    while(!mShutterSocket.getCurStatus(door_is_open)){//等待开门完成
        QCoreApplication::processEvents();
        QThread::msleep(50);
    }
    logNormal("请取出产品，并确认点胶是否OK");
    if(msgBox("手动确认是否OK",5) == "NG"){
        m_errorCode = -2;
        m_errMsg = "产品点胶固化NG";
        addLog(m_errMsg, false);
    }

}

void DispensingCuring::finish()
{
    QString out;
    if (!mBoolMesIgnore && isOnlineMode()) {
        emit logAddNormalLog("进行出站操作");
        if (!mMesObj.outProduce((m_errorCode == 0) ? 0 : 1, out)) {
            m_errMsg = "出站失败:" + out;
            emit logAddFailLog("出站失败:" + out);
            showFail("MES出站失败");
        } else {
            emit logAddNormalLog("出站完成:" + out);
        }
    }

    if (m_errorCode == 0) {
        emit resultShowPass();
    } else {
        emit resultShowFail(m_errMsg);
    }

    mShutterSocket.close();

    msgBox("请更换产品，并按绿色按钮进入下一轮测试", 5);

    stopProcess();
}



void DispensingCuring::slotConnectBtnClicked()
{
    int iContinue{0};
    emit resultShowDefault();

    ConfigInfo::getInstance()->getValueString(m_StationName, "DisCurIP", m_DisCurIP);
    logNormal("DisCurIP: " + m_DisCurIP);
    bool isOk{false};
    for (int foo = 0; foo < 20; foo++) {
        if (CPingTest::pingTest(nullptr, m_DisCurIP.toLocal8Bit().data())) {
            logNormal("ping ip " + m_DisCurIP + " ok!");
            iContinue++;
            if (iContinue >= 3) {
                isOk = true;
                break;
            }
        } else {
            logNormal("ping ip " + m_DisCurIP + " failed!");
        }
        QThread::msleep(500);
    }
    if(!isOk){
        m_errorCode = -1;
        m_errMsg = "网络通信异常";
        emit resultShowFail(m_errMsg);
        return;
    }
    BaseProduce::slotConnectBtnClicked();
}
void DispensingCuring::slotStartWorkBtnClicked()
{
    emit logClearData();
    emit resultShowDefault();

    qInfo("start ");
    startProcess();
}
void DispensingCuring::slotStopWorkBtnClicked()
{
    updateStatus(0);
    stopProcess();
}


bool ShutterSocket::init(const QString ip, const int port)
{
    socket.connectToHost(ip, port);
    QElapsedTimer timer;
    timer.start();
    while (timer.elapsed() < 10000) {
        QCoreApplication::processEvents();
        if (socket.state() == QTcpSocket::ConnectedState) {
            isOpen = true;
            return true;
        }
    }
    return false;
}

bool ShutterSocket::startDispensing()
{
    const char cmd[]{0x00 ,0x00 ,0x00 ,0x00 ,0x00 ,0x06 ,0x01 ,0x05 ,0x24 ,0x88 ,0xFF ,0x00};
    auto ret = socket.write(cmd, sizeof(cmd));
    if(ret <= 0) return false;
}

bool ShutterSocket::startCuring()
{
    const char cmd[]{0x00 ,0x00 ,0x00 ,0x00 ,0x00 ,0x06 ,0x01 ,0x05 ,0x24 ,0x8A ,0xFF ,0x00};
    auto ret = socket.write(cmd, sizeof(cmd));
    if(ret <= 0) return false;
}

bool ShutterSocket::getCurStatus(const signal_type_e& state)
{
    QElapsedTimer timer;
    timer.start();
    while (timer.elapsed() < 50000) {
        QThread::msleep(2000);
        if(socket.waitForReadyRead()){
            QByteArray ba = socket.readAll();
            if(ba.size() < 6) continue;
            char *buf = ba.data();
            if(buf[4] == state) return true;
        }
    }
    return false;
}

bool ShutterSocket::getIsOpen() const
{
    return isOpen;
}

bool ShutterSocket::close()
{
    if(getIsOpen()){
        socket.close();
        return true;
    }
    return false;
}

#include "dispensingcuring.h"
#include <QThread>
#include <CPingTest.h>
#include <QSettings>
#include "StaticSignals.h"
#include <QTextCodec>

DispensingCuring::DispensingCuring()
{
    connect(pMachine(), &QStateMachine::stopped,
            this, &DispensingCuring::slotStartWorkBtnClicked);

    m_StationName = ConfigInfo::getInstance()->cacheInfo().sCodeId;
    qDebug() << "Current Station Name: " << m_StationName;
    mMesObj.setUser(MesClient::instance()->sUser());
    int step{0};

    if(ConfigInfo::getInstance()->cacheInfo().sCodeId == "CL1003"){//快门点胶
        add(step++, "init_CL1003", std::bind(&DispensingCuring::init_CL1003, this));
    }else if(stationName() == "CL1005" || stationName() == "DE2001" || stationName() == "DE2002"){//镜头点胶
        add(step++, "init_CL1005", std::bind(&DispensingCuring::init_CL1005, this));
    }

    add(step++, "Judge", std::bind(&DispensingCuring::jugdeCustomLifeTime, this, true));

    if(ConfigInfo::getInstance()->cacheInfo().sCodeId == "CL1003"){
        add(step++, "doWork_CL1003", std::bind(&DispensingCuring::doWork_CL1003, this));
    }else if(stationName() == "CL1005" || stationName() == "DE2001" || stationName() == "DE2002"){
        add(step++, "doWork_CL1005", std::bind(&DispensingCuring::doWork_CL1005, this));
    }

    if(ConfigInfo::getInstance()->cacheInfo().sCodeId == "CL1003"){
        add(step++, "finish_CL1003", std::bind(&DispensingCuring::finish_CL1003, this));
    }else if(stationName() == "CL1005" || stationName() == "DE2001" || stationName() == "DE2002"){
        add(step++, "finish_CL1005", std::bind(&DispensingCuring::finish_CL1005, this));
    }

}

DispensingCuring::~DispensingCuring()
{
    m_lensDisSerial.close();
    m_lensDisSerial.freeHandle();
    m_lensCurSerial.close();
    m_lensCurSerial.freeHandle();
}


void DispensingCuring::init_CL1003()
{
    m_sn.clear();
    m_errorCode = 0;
    m_errMsg.clear();

    QString path = m_imgPath + "/SN0.png";
    StaticSignals::getInstance()->statusUpdateImg(path);

    m_sn = msgBox("获取产品SN");
    emit logAddNormalLog("产品SN:" + m_sn);
    setCurrentBarcode(m_sn); // 显示到界面
    enterMes(m_sn);
}

void DispensingCuring::doWork_CL1003()
{
    ShutterSocket socket;
    m_time_CL003.start();
    if(!socket.init(m_DisCurIP)){
        m_errMsg = "网络连接失败";
        m_errorCode = -1;
    }
    addLog("放入产品后，请手动关闭左右门");
    QThread::msleep(500);
    while(!socket.getCurStatus(door_is_close)){//等待关门完成
        QThread::msleep(300);
    }
    addLog("左右门已关闭，正在点胶中");
    if(!socket.startDispensing()){//开始点胶
        m_errorCode = -2;
        m_errMsg = "发送点胶信号失败";
        addLog(m_errMsg, false);
        return;
    }
    addLog("等待点胶完成");
    while(!socket.getCurStatus(dispensing_is_over)){//等待点胶完成
        QThread::msleep(300);
    }
    if(!socket.startCuring()){//开始固化
        m_errorCode = -2;
        m_errMsg = "发送固化信号失败";
        addLog(m_errMsg, false);
        return;
    }
    addLog("点胶已完成，正在固化中");
    while(!socket.getCurStatus(curing_is_over)){//等待固化完成
        QThread::msleep(300);
    }
    addLog("固化已完成，等待开门信号");
    while(!socket.getCurStatus(door_is_open)){//等待开门完成
        QThread::msleep(300);
    }

    addLog("左右门已打开，请取出产品，并确认点胶是否OK");
    DisplayPicture_CL1003();
    if(msgBox("手动确认是否OK",5) == "NG"){
        m_errorCode = -2;
        m_errMsg = "产品点胶固化NG";
        addLog(m_errMsg, false);
        return;
    }
}

void DispensingCuring::finish_CL1003()
{
    outMes();

    if (m_errorCode == 0) {
        emit resultShowPass();
    } else {
        emit resultShowFail(m_errMsg);
    }

    DisplayResult_CL1003();
    msgBox("请更换产品，并按绿色按钮进入下一轮测试", 5);

    stopProcess();
}


void DispensingCuring::init_CL1005()
{
    m_sn.clear();
    m_errorCode = 0;
    m_errMsg.clear();
    m_StartTime = "";
    m_EndTime = "";
    m_ConsumeTime = "";
    m_DisResult_CL1005.clear();

    QString path = m_imgPath + "/SN0.png";
    StaticSignals::getInstance()->statusUpdateImg(path);

    // 开启串口资源
    bool rst{false};
    rst = m_lensDisSerial.open(m_DisCom, 9600, true);
    if(!rst){
        m_lensDisSerial.close();
        m_lensDisSerial.freeHandle();
        m_errorCode = -1;
        m_errMsg = "开启点胶串口失败";
        return;
    }
    if(stationName() == "CL1005"){
        rst&= m_lensCurSerial.open(m_CurCom, 9600, true);
        if(!rst){
            m_lensCurSerial.close();
            m_lensCurSerial.freeHandle();
            m_errorCode = -1;
            m_errMsg = "开启固化串口失败";
            return;
        }
    }

    addLog("串口初始化成功");
}

void DispensingCuring::doWork_CL1005()
{
    //1.等待点胶请求
    addLog("等待点胶信号");
    while(!m_lensDisSerial.isStart()){
        QThread::msleep(300);
    }

    //2.响应点胶请求
    addLog("收到点胶信号，正在响应请求");
    if(!m_lensDisSerial.sendReply()){
        m_errorCode = -2;
        m_errMsg = "发送点胶响应信息失败";
        return;
    }
    //3.等待点胶结果
    addLog("已响应请求，等待点胶结果");
    while(!m_lensDisSerial.getResult(m_DisResult_CL1005)){
        QThread::msleep(300);
    }

    //4.响应点胶结果
    addLog("收到点胶结果，正在响应请求");
    if(!m_lensDisSerial.sendReply()){
        m_errorCode = -2;
        m_errMsg = "发送点胶结果响应信息失败";
        return;
    }

    if (stationName() == "CL1005") {
        //5.等待固化请求
        addLog("收到点胶结果，等待固化信号");
        while(!m_lensCurSerial.isStart()){
            QThread::msleep(300);
        }

        //6.响应固化请求
        addLog("收到固化信号，正在响应请求");
        if(!m_lensCurSerial.sendReply()){
            m_errorCode = -2;
            m_errMsg = "发送固化响应信息失败";
            return;
        }

        //7.开始统计时间，等待固化结果
        m_StartTime = QTime::currentTime().toString("hh:mm:ss");
        addLog("已响应请求，等待固化结果");
        while(!m_lensCurSerial.getResult(m_ConsumeTime)){
            QThread::msleep(300);
        }
        m_EndTime = QTime::currentTime().toString("hh:mm:ss");

        //8.响应固化结果
        addLog("收到固化结果，正在响应请求");
        if(!m_lensCurSerial.sendReply()){
            m_errorCode = -2;
            m_errMsg = "发送固化结果响应信息失败";
            return;
        }
    }
}

void DispensingCuring::finish_CL1005()
{
    for(auto i = 0;i < m_DisResult_CL1005.size();++i){
        QString sn = m_DisResult_CL1005.at(i).sn;
        QString result = m_DisResult_CL1005.at(i).disRet.contains("OK") == true ? "PASS&PASS" : "FAIL&FAIL";
        enterMes(sn);
        qDebug() << "result" << result;
        updateTable_CL1005(sn,"A1",m_StartTime,m_EndTime,m_ConsumeTime,m_NormTime,result);
        bool bResult = result.contains("PASS");
        outMes(bResult);
    }

    DisplayResult_CL1005();
    if (m_errorCode == 0) {
        emit resultShowPass();
    } else {
        emit resultShowFail(m_errMsg);
    }

    m_lensCurSerial.close();
    m_lensDisSerial.close();
    msgBox("请更换产品，并按Enter进入下一轮测试", 4);

    stopProcess();
}

void DispensingCuring::updateTable_CL1005(QString sn, QString zpm,
                                          QString start_time, QString end_time,
                                          QString consume_time, QString norm_time, QString result)
{
    emit tableAddRowData(QStringList() << sn << zpm << start_time << end_time << consume_time << norm_time << result);

    if(result.contains("FAIL")) {
        emit tableUpdateTestStatus(1);
    } else {
        emit tableUpdateTestStatus(0);
    }
}

void DispensingCuring::DisplayResult_CL1005()
{
    for(int i=0; i<m_tipsList.size(); i++) {
        addLog(m_tipsList[i].tips);
        StaticSignals::getInstance()->statusUpdateImg(m_tipsList[i].path);
        QThread::msleep(500);
    }
}


#define GETCONFIG(key) ConfigInfo::getInstance()->getValueString(key)
void DispensingCuring::DisplayPicture_CL1003()
{
    for(int i=0; i<m_tipsList.size(); i++) {
        addLog(m_tipsList[i].tips);
        StaticSignals::getInstance()->statusUpdateImg(m_tipsList[i].path);
//        updateTable_CL1003(m_tipsList[i].tips, m_time_CL003.elapsed(), m_errorCode == 0);
        QThread::msleep(500);
    }
}
void DispensingCuring::DisplayResult_CL1003()
{
    for(int i=0; i<m_tipsList.size(); i++) {
        addLog(m_tipsList[i].tips);
//        StaticSignals::getInstance()->statusUpdateImg(m_tipsList[i].path);
        updateTable_CL1003(m_tipsList[i].tips, m_time_CL003.elapsed(), m_errorCode == 0);
        QThread::msleep(500);
    }
}
void DispensingCuring::updateTable_CL1003(QString name, qint64 workTime, bool result)
{
    if(result) {
        emit tableAddRowData(QStringList() << name << QString::number(workTime) << "PASS");
        emit tableUpdateTestStatus(0);
    } else {
        emit tableAddRowData(QStringList() << name << QString::number(workTime) << "FAIL");
        emit tableUpdateTestStatus(1);
    }
}



void DispensingCuring::enterMes(QString sn)
{
    if (isOnlineMode()) {
        QString out;
        addLog("入站:" + sn);
        if (!mMesObj.enterProduce(sn, out)) {
            m_errMsg = "入站失败: " + out ;
            emit logAddFailLog("入站失败:" + out);
            mBoolMesIgnore = true;
            m_errorCode = -1;
            return;
        }
        addLog("入站:" + sn + " ok");
        mBoolMesIgnore = false;

    } else {
        logWarming("当前为离线模式,不进行入站操作");
    }
}
void DispensingCuring::outMes(bool result)
{
    QString out;
    if (!mBoolMesIgnore && isOnlineMode()) {
        emit logAddNormalLog("进行出站操作");
        if (!mMesObj.outProduce((m_errorCode == 0 || result) ? 0 : 1, out)) {
            m_errMsg = "出站失败:" + out;
            emit logAddFailLog("出站失败:" + out);
            showFail("MES出站失败");
        } else {
            emit logAddNormalLog("出站完成:" + out);
        }
    }
}


void DispensingCuring::slotConnectBtnClicked()
{
    int iContinue{0};
    emit resultShowDefault();

    QString produceName = ConfigInfo::getInstance()->getValueString("项目选择");
    QString configPath = QCoreApplication::applicationDirPath() + QString("/config/guidecfg_%1.ini").arg(produceName);

    QSettings reader(configPath, QSettings::IniFormat);
    reader.setIniCodec(QTextCodec::codecForName("UTF-8"));
    reader.beginGroup("config");

    QString defaultPath = QCoreApplication::applicationDirPath() + "/res/";
    m_imgPath = reader.value("imgPath", QVariant(defaultPath)).toString();

    ComGuideCfg::getGuideCfgList(m_tipsList);

    if(ConfigInfo::getInstance()->cacheInfo().sCodeId == "CL1003"){//快门点胶
        emit tableAddHeader(QStringList() << "检测项" << "工作时间(ms)" << "结果");
        ConfigInfo::getInstance()->getValueString(m_StationName, "CL1003点胶固化IP", m_DisCurIP);
        addLog("点胶固化IP: " + m_DisCurIP);
        bool isOk{false};
        for (int foo = 0; foo < 20; foo++) {
            if (CPingTest::pingTest(nullptr, m_DisCurIP.toLocal8Bit().data())) {
                addLog("ping ip " + m_DisCurIP + " ok!");
                iContinue++;
                if (iContinue >= 3) {
                    isOk = true;
                    break;
                }
            } else {
                addLog("ping ip " + m_DisCurIP + " failed!");
            }
            QThread::msleep(500);
        }
        if(!isOk){
            m_errorCode = -1;
            m_errMsg = "网络通信异常";
            emit resultShowFail(m_errMsg);
            return;
        }
    }else if(stationName() == "CL1005" || stationName() == "DE2001" || stationName() == "DE2002"){//镜头点胶
        emit tableAddHeader(QStringList() << "SN" << "载盘码" << "固化开始时间" << "固化结束时间" << "固化耗费时间(s)" << "标准固化时间(s)" << "点胶&固化结果");

        ConfigInfo::getInstance()->getValueString(m_StationName, "点胶机串口", m_DisCom);
        addLog("点胶机串口:"+m_DisCom);
        bool rst = m_lensDisSerial.open(m_DisCom,9600,true);
        if(!rst){
           m_lensDisSerial.close();
           m_lensDisSerial.freeHandle();
        }
        addLog("点胶机串口环境检测OK");
        m_lensDisSerial.close();

        if(stationName() == "CL1005"){
            ConfigInfo::getInstance()->getValueString(m_StationName, "固化机串口", m_CurCom);
            ConfigInfo::getInstance()->getValueString(m_StationName, "标准固化时间", m_NormTime);
            addLog("固化机串口:"+m_CurCom);
            addLog("标准固化时间:"+m_NormTime);
            rst&= m_lensCurSerial.open(m_CurCom,9600,true);
            if(!rst){
                m_lensCurSerial.close();
                m_lensCurSerial.freeHandle();
                showFail("开启串口失败");
                return;
            }
            addLog("固化机串口环境检测OK");
            m_lensCurSerial.close();
        }

    }
    BaseProduce::slotConnectBtnClicked();
}
void DispensingCuring::slotStartWorkBtnClicked()
{
    emit tableClearData();
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



//快门点胶固化网络操作
ShutterSocket::~ShutterSocket()
{
    socket.disconnectFromHost();
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


//镜头点胶串口操作
bool LensDisSerial::isStart()
{
    if (!isOpened()) {
        qDebug() << ">> 点胶串口未打开";
        return false;
    }
    const char* cmd{"ST"};
    if(!find((uint8_t*)cmd,2,500)){
        qDebug() << ">> 获取点胶开始信号失败";
        return false;
    }
    return true;
}

bool LensDisSerial::sendReply()
{
    if (!isOpened()) {
        return false;
    }
    const char* cmd{"ACK"};
    return write((uint8_t*)cmd,3);
}

bool LensDisSerial::getResult(QList<lens_result_s> &result)
{
    int len{0};
    char retRst[1024]{0};
    char* pRst = (char*)retRst;
    result.clear();
    if (!get(nullptr, 0, nullptr, 0, pRst, len,500)) {
        qDebug() << ">> 获取点胶结果返回数据失败";
        return false;
    }
    qDebug("Recv Data Len:%d",len);

    if(len < 15){
        qDebug() << ">> 数据长度异常";
        return false;
    }

    QString strRst = QString(pRst);//格式：  数量    SN      扫码结果 点胶结果。。。。。。。
                                   //       16;A00076AABXL0158,OK,检测OK;A00076AABXL0159,OK,检测OK;.......................
    qDebug() << strRst;
    QStringList strlistRst = strRst.split(';');
    int size = QString(strlistRst.at(0)).toInt();//产品数量
    if(size > strlistRst.size()-1 || size <=0 ){
        qDebug() << ">> 获取点胶结果数量异常";
        qDebug(" >> 应该返回%d, 实际返回:%d",size,strlistRst.size()-1);
        return false;
    }
    qDebug() << strlistRst;
    for(auto i = 1;i <= size;++i){
        qDebug() << strlistRst.at(i);
        lens_result_s res;
        QStringList list = QString(strlistRst.at(i)).split(',');
        if(list.size() < 3){
            qDebug() << list;
            qDebug() << ">> 点胶结果解析错误，数据格式不一致";
            result.clear();
            return false;
        }

        res.sn = QString(list.at(0));
        res.scanRet = QString(list.at(1));
        res.disRet = QString(list.at(2));

        result.append(res);
    }
    return true;
}

//镜头固化串口操作
bool LensCurSerial::isStart()
{
    if (!isOpened()) {
        return false;
    }
    const char* cmd{"#ST"};
    if(!find((uint8_t*)cmd,3,500)){
        qDebug() << ">> 获取点胶开始信号失败";
        return false;
    }
    return true;
}

bool LensCurSerial::sendReply()
{
    if (!isOpened()) {
        return false;
    }
    const char* cmd{"ACK"};
    return write((uint8_t*)cmd,3);
}

bool LensCurSerial::getResult(QString &time)
{
    int len{0};
    char retRst[32]{0};
    char* pRst = (char*)retRst;
    if (!get((char*)("CureTime:"), 9, nullptr, 0, pRst, len,500)) {
        qDebug() << ">> 获取固化结果返回数据失败";
        return false;
    }
    qDebug("Len:%d",len);
    if(len < 3){
        qDebug() << ">> 数据长度异常";
        return false;
    }

    QString strRet = QString(pRst);//格式：#CureTime:10s
    QStringList list = strRet.split(':');
    if(list.size() < 2){
        qDebug() << "解析固化耗时失败";
    }
    QString strTime = QString(list.at(1));
    time = strTime.left(strTime.size()-1);
    return true;
}

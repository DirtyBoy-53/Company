#include "armcb1002.h"

#include "autoprocessguide.h"
#include "StaticSignals.h"
#include <QTime>
#include "util.h"
#include "videoinfo.h"
#include <QJsonObject>
#include <QJsonArray>
#include <QJsonDocument>
#include <QElapsedTimer>

ArmCb1002::ArmCb1002()
{
    initStateMachine();
    m_http = new HttpManager();
    MSG_FUN_HANDLE fun = std::bind(&ArmCb1002::handleReq,
                                   this,
                                   std::placeholders::_1,
                                   std::placeholders::_2,
                                   std::placeholders::_3);
    m_http->setHandler(fun);
    m_http->Start();
}

void ArmCb1002::initTipsInfo()
{
    ComGuideCfg::getGuideCfgList(m_tipsList);
}


void ArmCb1002::slotConnectBtnClicked()
{
    emit tableAddHeader(QStringList() <<"名称"<< "标浮" << "实浮" <<"扭力"<< "圈数"<<"压力"<< "工作时间(ms)" << "结果");

    BaseProduce::slotConnectBtnClicked();
    // addLog("点击开始工作按钮");
    // updateStatus(1);
}

void ArmCb1002::initStateMachine()
{
    connect(pMachine(), &QStateMachine::stopped,
            this, &ArmCb1002::slotsStoped);

    add(0, "check next", std::bind(&ArmCb1002::checkNext, this));

    add(10, "get sn", std::bind(&ArmCb1002::getSn, this));
    add(11, "enter mes", std::bind(&ArmCb1002::enterMes, this));

    add(15, "get len sn", std::bind(&ArmCb1002::getLenSn, this));

    add(30, "tips work", std::bind(&ArmCb1002::tipsWork, this));
    add(35, "waitdata", std::bind(&ArmCb1002::waitData, this));
    add(36, "waitdata", std::bind(&ArmCb1002::waitPaste, this));
    add(37, "waitdata", std::bind(&ArmCb1002::waitData2, this));

    add(40, "upload mes", std::bind(&ArmCb1002::mesUpload, this));
    add(50, "out mes", std::bind(&ArmCb1002::outMes, this));
    add(60, "stop", std::bind(&ArmCb1002::stopWork, this));
}

void ArmCb1002::checkNext()
{
//    if(m_errorCode != 0) {
//        msgBox("FAIL 请将不良品放入不良品盒", 3);
//    }

    initTipsInfo();

    logClear();

    showProcess("");

    m_errorCode = 0;
    m_frameCount = 0;
    m_waitint = 0;
    m_bResultCode = true;
    for(int i = 0;i < SNEWCOUNT;i ++) {
        SnewInfo &info = m_snews[i];
        info.clear();
        info.code = "第" + QString::number(i+1) + "颗螺丝";
    }
}

void ArmCb1002::handleReq(std::string sUri, std::string &sIn, std::string &sOut)
{
    qInfo()<<"recv "<< QString::fromStdString(sUri) << QString::fromStdString(sIn);
    QJsonObject jOut;
    if(1 != m_waitint) {
        qInfo("status error..");
        jOut.insert("stage", m_stepNumber);
        jOut.insert("error_code", 1);
        jOut.insert("msg", "status error...");
        sOut = QString(QJsonDocument(jOut).toJson(QJsonDocument::Compact)).simplified().toStdString();
        return;
    }


    qInfo()<<"recv "<< QString::fromStdString(sUri) << QString::fromStdString(sIn);
    if(sUri.find("/api/user/available") != std::string::npos) {
        addLog("接收到机器锁附请求");
        QJsonParseError jsonError;
        QJsonDocument document = QJsonDocument::fromJson(sIn.data(), &jsonError);
        if(document.isNull() || !document.isObject() || jsonError.error != QJsonParseError::NoError) {
            jOut.insert("stage", m_stepNumber);
            jOut.insert("error_code", 1);
            jOut.insert("msg", "msgparse error");
            sOut = QString(QJsonDocument(jOut).toJson(QJsonDocument::Compact)).simplified().toStdString();
            m_waitint = 0;
            m_errorCode = -1;
            addLog("接收到机器锁附请求，参数异常", -1);
            qInfo("parse error....");
            return ;
        }
        QJsonObject map = document.object();
        if(!map.contains("stage")) {
            jOut.insert("stage", m_stepNumber);
            jOut.insert("error_code", 1);
            jOut.insert("msg", "msgparse error");
            sOut = QString(QJsonDocument(jOut).toJson(QJsonDocument::Compact)).simplified().toStdString();
            m_waitint = 0;
            qDebug() << "m_waitint = 0 2";
            logAddFailLog("当前请求不包含步骤信息，当前步骤应为：" + QString::number(m_stepNumber));
            return;
        }
        int stage = map["stage"].toInt();
        logAddNormalLog("当前请求阶段为：" + QString::number(stage));
        if (stage != m_stepNumber) {
            jOut.insert("stage", m_stepNumber);
            jOut.insert("error_code", 1);
            jOut.insert("msg", "steps error");
            sOut = QString(QJsonDocument(jOut).toJson(QJsonDocument::Compact)).simplified().toStdString();
            m_waitint = 0;
            qDebug() << "m_waitint = 0 3";
            logAddFailLog("请求步骤与当前步骤不一致，当前步骤为：" + QString::number(m_stepNumber));
            return;
        }

        int errCode = (m_errorCode == 0) ? 0 : 1;
        jOut.insert("stage", m_stepNumber);
        jOut.insert("error_code", errCode);
        jOut.insert("msg", "suc");
        sOut = QString(QJsonDocument(jOut).toJson(QJsonDocument::Compact)).simplified().toStdString();
        qInfo("ok...");
    } else if(sUri.find("/api/user/report") != std::string::npos) {
        logAddNormalLog("接收到锁附结果报告");
        int iErr = 1;
        QJsonParseError jsonError;
        QJsonDocument document = QJsonDocument::fromJson(sIn.data(), &jsonError);
        if(document.isNull() || !document.isObject() || jsonError.error != QJsonParseError::NoError) {
            jOut.insert("stage", m_stepNumber);
            jOut.insert("error_code", 1);
            jOut.insert("msg", "msgparse error");
            sOut = QString(QJsonDocument(jOut).toJson(QJsonDocument::Compact)).simplified().toStdString();
            m_waitint = 0;
            qDebug() << "m_waitint = 0 4";
            qInfo("parse error....");
            return ;
        }

        QJsonObject map = document.object();
        if(map.contains("items") && map.value("items").isArray()) {
            QJsonArray arr = map.value("items").toArray();
            for(int i = 0;i < arr.size();i ++) {
                if(i >= SNEWCOUNT) {
                    break;
                }
                QJsonObject obj = arr.at(i).toObject();
                if(obj.contains("data" + QString::number(i + 1))) {
                    SnewInfo &info = m_snews[m_curSnewsIdx++];
                    qInfo()<<obj.value("data" + QString::number(i + 1)).toString();
                    QStringList ele = obj.value("data" + QString::number(i + 1)).toString().split(",");
                    if(ele.size() >= 1)
                        info.biaofu = ele.at(0);
                    if(ele.size() >= 2)
                        info.shifu = ele.at(1);
                    if(ele.size() >= 3)
                        info.fucha = ele.at(2);
                    if(ele.size() >= 4)
                        info.niuli = ele.at(3);
                    if(ele.size() >= 5)
                        info.quanshu = ele.at(4);
                    if(ele.size() >= 6)
                        info.yali = ele.at(5);
                    if(ele.size() >= 7) {
                        info.result = ele.at(6);
                        if (info.result.isEmpty()) {
                            info.result = "NG";
                        }
                    }
                    if(ele.size() >= 8)
                        info.sn = ele.at(7);
                    iErr = 0;
                }else {
                    qInfo("not found data%d",i + 1);
                }
            }

        }else {
            qInfo("not found items");
        }
        jOut.insert("stage", m_stepNumber);
        if(iErr) {
            jOut.insert("error_code", 1);
            jOut.insert("msg", "err");
        }else {
            jOut.insert("error_code", 0);
            jOut.insert("msg", "suc");
        }
        sOut = QString(QJsonDocument(jOut).toJson(QJsonDocument::Compact)).simplified().toStdString();
        m_waitint = 0;
    }
}

void ArmCb1002::showItem(SnewInfo item)
{
    qDebug() << "名称"<< "标浮" << "实浮" <<"扭力"<< "圈数"<<"压力"<< "工作时间(ms)" << "结果";
    QStringList itemString;
    itemString << item.code;
    itemString << item.biaofu;
    itemString << item.shifu;
    itemString << item.niuli;
    itemString << item.quanshu;
    itemString << item.yali;

    itemString << item.sTm;
    itemString << item.result ;

    tRowData(itemString);
    qDebug() << itemString;
    if(item.result == MESPASS) {
        tableUpdateTestStatus(0);
    }else{
        tableUpdateTestStatus(1);
    }
}

void ArmCb1002::showImage2UI(QString name)
{
    QString path = QCoreApplication::applicationDirPath() + "/res/CB1002/";
    StaticSignals::getInstance()->statusUpdateImg(path + name);
}

void ArmCb1002::slotStartWorkBtnClicked()
{
    tClear();
    m_errMsg.clear();
    logClear();
    QDateTime dateTime = QDateTime::currentDateTime();
    m_bResultCode = true;
    m_bworkStatus = true;
    startProcess();
}

void ArmCb1002::slotStopWorkBtnClicked()
{
    updateStatus(0);
    stopProcess();
    m_bworkStatus = false;
    m_signal.signal();
    hideMsgBox();
    showProcess("请先点检环境是否正常");
}

void ArmCb1002::stopWork()
{
    QString path = logPath() + "/" + QDate::currentDate().toString("yyyyMMdd");
    QDir dir(path);
    if (!dir.exists(path)) {
        dir.mkpath(path);
    }

    path = path + "/" + (m_errorCode == 0 ? "PASS_" : "FAIL_") + m_sn + "_" + QTime::currentTime().toString("hhmmss");
    tableSaveCsv(path + ".csv");
    logSaveTxt(path + ".txt");

    if (m_errorCode == 0) {
        showPass();
    } else {
        showFail("组装失败，errorCode: " + QString::number(m_errorCode));
    }


    msgBox("请替换产品进行下一轮测试", 4);
    if(m_bworkStatus) {

    }
    stopProcess();
}


void ArmCb1002::tipsWork()
{
    if(m_errorCode != 0) return;

    bool useSelector = false;
    ConfigInfo::getInstance()->getValueBoolean("GLOBAL", "按钮选择器", useSelector);
    for(int i=0; i<m_tipsList.size(); i++) {
        QTime timer;
        timer.start();

        addLog(m_tipsList[i].tips);
        StaticSignals::getInstance()->statusUpdateImg(m_tipsList[i].path);
        msgBox(m_tipsList[i].msgTips, 3);
        updateTable(m_tipsList[i].tips, timer.elapsed(), true);
        if (useSelector) {
            QThread::msleep(500);
        }
    }
}

void ArmCb1002::addLog(QString log, const int  result)
{
    if(result == -1) {
        logAddFailLog(log);
        showFail(log);
    } else if(result == 1) {
        logAddPassLog(log);
        showPass();
    } else {
        logAddNormalLog(log);
        showProcess(log);
    }
}

void ArmCb1002::updateTable(QString name, qint64 workTime, bool result)
{
    QStringList itemString;
    itemString << name;
    itemString << "/";
    itemString << "/";
    itemString << "/";
    itemString << "/";
    itemString << "/";

    itemString << QString::number(workTime);
    itemString << (result?MESPASS:MESFAILED) ;

    tRowData(itemString);

    if(result) {
        emit tableUpdateTestStatus(0);
    } else {
        emit tableUpdateTestStatus(1);
    }
}

void ArmCb1002::slotsStoped()
{
    QString logPath = ConfigInfo::getInstance()->getValueString("LogPath");
    logPath += "/" + m_sn + ".csv";
    tableSaveCsv(logPath);
    addLog("停止工作");
    if(m_bworkStatus) {
        startProcess();
    }
}

void ArmCb1002::waitData()
{
    if (m_errorCode != 0) return;
    showImage2UI("CB1002_P2.png");
    showProcess("主板螺钉锁付");
    addLog("主板螺钉锁付");

    QElapsedTimer tStart;
    tStart.start();
    m_curSnewsIdx = 0;
    m_stepNumber = 0;
    m_waitint = 1;
    while(m_waitint != 0) {
        if(!m_bworkStatus) {
            break;
        }
        QThread::msleep(100);
    }
    for(int i = 0;i < 3;i ++) {
        SnewInfo &info = m_snews[i];

        if ((info.result == "OK")) {
            info.result = MESPASS;
        } else {
            info.result = MESFAILED;
            logFail("主板螺钉锁付 失败");
            m_errorCode = -1;
        }
        QString sRes = info.result;
        MesCheckItem tem[5];
        tem[0].sItem = "screw_no_" + QString::number(i + 1);
        tem[0].sResult = sRes;
        tem[0].sValue= QString::number(i + 1);
        tem[1].sItem = "screw_torque_force_" + QString::number(i + 1);
        tem[1].sResult = sRes;
        tem[1].sValue= info.niuli;
        tem[2].sItem = "screw_turns_nums_" + QString::number(i + 1);
        tem[2].sResult = sRes;
        tem[2].sValue= info.quanshu;
        tem[3].sItem = "screw_returns_nums_" + QString::number(i + 1);
        tem[3].sResult = sRes;
        tem[3].sValue= "1";
        tem[4].sItem = "screw_result_" + QString::number(i + 1);
        tem[4].sResult = sRes;
        tem[4].sValue= sRes;
        info.sTm = QString::number(tStart.elapsed());
        MesCom::instance()->addItemResultEx(tem[0]);
        MesCom::instance()->addItemResultEx(tem[1]);
        MesCom::instance()->addItemResultEx(tem[2]);
        MesCom::instance()->addItemResultEx(tem[3]);
        MesCom::instance()->addItemResultEx(tem[4]);
        showItem(info);
    }
}

void ArmCb1002::waitData2()
{
    if (m_errorCode != 0) return;

    showImage2UI("CB1002_P4.png");
    showProcess("下盖螺钉锁付");
    addLog("下盖螺钉锁付");

    QElapsedTimer tStart;
    tStart.start();
    m_curSnewsIdx = 0;
    m_stepNumber = 1;
    m_waitint = 1;
    while(m_waitint != 0) {
        if(!m_bworkStatus) {
            break;
        }
        QThread::msleep(100);
    }
    for(int i = 0;i < 4;i ++) {
        SnewInfo info = m_snews[i];
        if((info.result == "OK")) {
            info.result = MESPASS;
        }else {
            info.result = MESFAILED;
            m_errorCode = -1;
            logFail("下盖螺钉锁付 失败");
        }
        QString sRes = info.result;
        MesCheckItem tem[5];
        tem[0].sItem = "screw_no_" + QString::number(i + 1);
        tem[0].sResult = sRes;
        tem[0].sValue= QString::number(i + 1);
        tem[1].sItem = "screw_torque_force_" + QString::number(i + 1);
        tem[1].sResult = sRes;
        tem[1].sValue= info.niuli;
        tem[2].sItem = "screw_turns_nums_" + QString::number(i + 1);
        tem[2].sResult = sRes;
        tem[2].sValue= info.quanshu;
        tem[3].sItem = "screw_returns_nums_" + QString::number(i + 1);
        tem[3].sResult = sRes;
        tem[3].sValue= "1";
        tem[4].sItem = "screw_result_" + QString::number(i + 1);
        tem[4].sResult = sRes;
        tem[4].sValue= sRes;
        info.sTm = QString::number(tStart.elapsed());
        MesCom::instance()->addItemResultEx(tem[0]);
        MesCom::instance()->addItemResultEx(tem[1]);
        MesCom::instance()->addItemResultEx(tem[2]);
        MesCom::instance()->addItemResultEx(tem[3]);
        MesCom::instance()->addItemResultEx(tem[4]);
        showItem(info);
    }
}

void ArmCb1002::waitPaste()
{
    if (m_errorCode != 0) return;
    showImage2UI("CB1002_P3.png");
    showProcess("请粘贴导电泡棉");
    msgBox("请粘贴导电泡棉", 4);
}

void ArmCb1002::getSn()
{
    addLog("开始扫码 SN");
    showImage2UI("CB1002_P0.png");
    m_sn = msgBox("请扫码 SN");
    tClear();
    if(ConfigInfo::getInstance()->sysInfo().mode == 0) {
        CacheInfo info = ConfigInfo::getInstance()->cacheInfo();
        info.sSn = m_sn;
        ConfigInfo::getInstance()->setCacheInfo(info);
    }

    addLog("SN: " + m_sn);
}

void ArmCb1002::getLenSn()
{
    if(m_errorCode != 0) return;
    addLog("开始扫码 后盖编码");

    showImage2UI("CB1002_P1.png");

    QString code = msgBox("请扫码 后盖编码");
    addLog("获取后盖编码 " + code);

    MesCheckItem item;
    item.sItem = "box_back_sn";
    if(code.size() < 5) {
        addLog("获取后盖编码失败,长度不足", -1);
        m_errMsg = "获取后盖编码失败,长度不足";
        m_errorCode = -18;
        item.sResult = MESFAILED;
    } else {
        m_errMsg.clear();
        item.sResult = MESPASS;
    }
    item.sValue = code;
    MesCom::instance()->addItemResultEx(item);
}

void ArmCb1002::mesUpload()
{
    if(m_errorCode != 0) {
        m_bResultCode = false;
    } else {
        m_bResultCode = true;
    }
    if(!isOnlineMode()) {
        return;
    }
}

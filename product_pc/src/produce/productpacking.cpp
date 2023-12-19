#include "productpacking.h"
#include <functional>
#include <QMessageBox>
#include <inputmessagebox.h>
#include <QFile>
#include <CMyPrinter.h>
#include <QThread>
#include <util.h>
#include <QDateTime>
#include <QCoreApplication>
#include <QProcess>
#include <StaticSignals.h>
#include <QSettings>
#include <QDir>

static MesCheckItem g_mesCheckPackage[MESMAXCHECKNUM] = {
    //item             stardvalue minvalue maxvalue result
    {"sn", "", "","",""},
    {""},
};

productpacking::productpacking()
{
    connect(pMachine(), &QStateMachine::stopped, this, &productpacking::slotsStoped);
    connect(StaticSignals::getInstance(), &StaticSignals::packingNotFullPrint, this,  &productpacking::slotNotFullPrint);

    add(10, "init", std::bind(&productpacking::initParam, this));
    add(20, "beforepacking", std::bind(&productpacking::beforePacking, this));
    add(30, "packing", std::bind(&productpacking::packing, this));
    add(40, "afterpacking", std::bind(&productpacking::afterPacking, this));
    add(50, "deinit", std::bind(&productpacking::deInit, this));

}

void productpacking::slotprintSN()
{
   std::this_thread::sleep_for(std::chrono::milliseconds(1000));
   m_bResultCode=true;
//   std::thread([this]{
//      }).detach();
}

void productpacking::slotsStoped()
{
    qInfo("slotsStoped");
    slotStartWorkBtnClicked();
}

void productpacking::slotNotFullPrint()
{
    qDebug() << "Not full";
    if (m_pkgInfors.size() == 0) {
        showProcess("还没有任何条码，不执行打印");
        return;
    }

    m_printAnyway = true;

    hideMsgBox();
    m_signal.signal();
}

void productpacking::slotStartWorkBtnClicked()
{
    startProcess();
}

void productpacking::slotConnectBtnClicked()
{
    if (!getParam()) {
        //showFail("获取参数失败");
        return;
    }

    QStringList header;
    header << "产品序列号" << "单件重量(kg)" << "阶段总重量(不带包材,单位kg)" << "阶段总重量(带包材,单位kg)" << "结果";
    tableAddHeader(header);

    mLogPath.clear();
    mLogPath = logPath() + "/";
    mLogPath += QDate::currentDate().toString("hhmmss") + "/";
    QDir dir(mLogPath);
    if (!dir.exists()) {
        dir.mkpath(mLogPath);
    }

    if (!m_elecScale.getHandle()) {
        showFail("加载 DataLinkTransport 组件失败");
        return;
    }

    if (!m_elecScale.open(m_elecSerial, 9600, false)) {
        showFail("打开 电子称控制串口失败!");
        return;
    }
    m_elecScale.close();

    resultShowProcess("初始化OK，请点击开始工作按钮执行测试");
    BaseProduce::slotConnectBtnClicked();
}

bool productpacking::getParam()
{
    m_StationName = ConfigInfo::getInstance()->cacheInfo().sCodeId;

    m_mesObj.setUser(MesClient::instance()->sUser());


    ConfigInfo::getInstance()->getValueString(m_StationName, "电子秤串口", m_elecSerial);
    ConfigInfo::getInstance()->getValueString(m_StationName, "包装打印模板", m_printTemplatePath);
    ConfigInfo::getInstance()->getValueBoolean(m_StationName, "是否在线模板", m_onlineTemplate);
    logAddNormalLog("当前工位配置打印模板从MES获取：" + QString(m_onlineTemplate ? "True" : "False"));

    if (m_onlineTemplate) {
        m_printTemplatePath.clear();
        m_printTemplatePath = QCoreApplication::applicationDirPath() + "/";
        m_printTemplatePath += QTime::currentTime().toString("hhmmss") + ".btw";
        if (!m_mesObj.getPackagePrintFile(m_printTemplatePath)) {
            m_errMsg = "在线获取打印模板失败，请确认此工单是否有配置打印模板文件，或切换为本底打印模板获取";
            logAddFailLog(m_errMsg);
            showFail(m_errMsg);
            return false;
        }
    }

    QFileInfo fInfor(m_printTemplatePath);
    if (!fInfor.exists()) {
        m_errMsg = "模板文件不存在";
        logAddFailLog(m_errMsg);
        showFail(m_errMsg);
        return false;
    }
    logAddNormalLog("当前工位配置打印模板为：" + m_printTemplatePath);

    return true;
}

void productpacking::initParam()
{
    m_pkgInfors.clear();

    tableClearData();
    logClearData();
    showDefault();
    m_errMsg.clear();
    m_errorCode = 0;

    m_packBoxSN.clear();
    m_sn.clear();

    if (!isOnlineMode()) {
        m_errorCode = -1;
        m_errMsg = "当前为非在线模式，请切换配置页面模式";
        return;
    }

    ConfigInfo::getInstance()->getValueInt32(m_StationName, "包装产品总数", m_totalCount);
    ConfigInfo::getInstance()->getValueInt32(m_StationName, "包装产品层数", m_layoutCount);
    m_singleCount = m_totalCount / m_layoutCount;
    logAddNormalLog(QString("当前包装总数：%1，包装层数为：%2，单层需包装：%3").arg(m_totalCount).arg(m_layoutCount).arg(m_singleCount));
    logAddNormalLog("当前工位配置打印模板为：" + m_printTemplatePath);
    CacheInfo cInfo = ConfigInfo::getInstance()->cacheInfo();
    m_workOrder = cInfo.orderId;

    QString data = msgBox("请确认工单号：" + m_workOrder + "是否正确!!!", MSGBOX_SELECT);
    if (data != "PASS") {
        m_errorCode = -4;
        m_errMsg = "当前工单号不对，请在菜单栏进行工单切换!";
        return;
    }

    logAddWarningLog("当前所配置工单号：" + m_workOrder);

    if (!m_elecScale.open(m_elecSerial, 9600, false)) {
        m_errorCode = -2;
        m_errMsg = "开启电子秤串口失败";
        return;
    }
}


void productpacking::beforePacking()
{
    if (m_errorCode != 0) return;

    if (0 != m_mesObj.getOrderSns(m_workOrder, m_packBoxSN, true)) {
        m_errMsg = "获取包箱条码失败: " + m_packBoxSN;
        logAddFailLog(m_errMsg);
        m_errorCode = -1;
        return;
    } else {
        showProcessLog("获取到包箱SN为: " + m_packBoxSN);
    }

    m_workAACode = m_mesObj.getMaterialAACode(m_workOrder);
    logAddWarningLog("当前工单AA码：" + m_workAACode);
    //
    showProcessLog("请保持电子秤空载，即将进行清零操作");
    msgBox("请保持电子秤空载，即将进行清零操作", 4);
    m_elecScale.setQP();
    m_elecScale.setGL();
}

bool productpacking::snValidCheck(QString sn)
{
    int snNeedLen = snGetLength();
    QString snPrefix = snGetPrefix();
    if (snNeedLen > 0) {
        if (sn.length() != snNeedLen) {
            QString tips = "SN长度匹配错误，需求SN长度为" + QString::number(snNeedLen);
            tips += ", 请重新扫码或更换产品扫码";
            showProcessLog(tips);
            return false;;
        }
        if (snPrefix.size() != 0) {
            if (sn.indexOf(snPrefix) != 0) {
                QString tips = "SN前缀匹配错误，需求SN前缀为" + snPrefix;
                tips += ", 请重新扫码或更换产品扫码";
                showProcessLog(tips);
                return false;;
            }
        }
    }

    return true;
}

bool productpacking::heavyValidCheck(WeightingType wt, float weight)
{
    QString rules;
    QString logs;
    switch (wt) {
    case WTypeSingleWeight:
        logs.append("单台产品,");
        ConfigInfo::getInstance()->getValueString(m_StationName, "单台产品重量下限与上限", rules);
        break;
    case WTypeLineFoam:
        logs.append("内衬泡棉,");
        ConfigInfo::getInstance()->getValueString(m_StationName, "内衬泡棉重量下限与上限", rules);
        break;
    case WTypeCoverFoam:
        logs.append("上盖泡棉,");
        ConfigInfo::getInstance()->getValueString(m_StationName, "泡棉盖重量下限与上限", rules);
        break;
    case WTypeNakedWeight:
        logs.append("裸机总重,");
        ConfigInfo::getInstance()->getValueString(m_StationName, "裸机总重量下限与上限", rules);
        break;
    case WTypeFullWeight:
        logs.append("包装总重,");
        ConfigInfo::getInstance()->getValueString(m_StationName, "包装总重量下限与上限", rules);
        break;
    }
    QStringList rulesArray = rules.split(";");
    if (rulesArray.size() != 3) {
        showProcessLog("重量的配置文件格式不合法");
        return false;
    }

    QString rSwitch = rulesArray.at(0);
    // bool isCheck = ((rSwitch.toInt() == 1) ? true : false);
    if (rSwitch.indexOf("1") == -1) {
        showProcessLog("配置为不执行检测，返回true，配置为：" + rSwitch);
        return true;
    } else {
        float min = rulesArray[1].toFloat();
        float max = rulesArray[2].toFloat();
        if ((WTypeNakedWeight == wt) && m_printAnyway) {
            QString myRules;
            ConfigInfo::getInstance()->getValueString(m_StationName, "单台产品重量下限与上限", myRules);
            QStringList arrays = myRules.split(";");
            if (arrays.size() != 3) {
                showProcessLog("Get重量的配置文件格式不合法");
                return false;
            }
            float singleHeavy = (arrays[1].toFloat() + arrays[2].toFloat()) / 2;
            int count = m_pkgInfors.size();
            min = count * singleHeavy - fabs(max - min) / 2;
            max = count * singleHeavy + fabs(max - min) / 2;
        }

        if ((WTypeFullWeight == wt) && m_printAnyway) {
            QString myRules;
            ConfigInfo::getInstance()->getValueString(m_StationName, "单台产品重量下限与上限", myRules);
            QStringList arrays = myRules.split(";");
            if (arrays.size() != 3) {
                showProcessLog("Get重量的配置文件格式不合法");
                return false;
            }

            float singleHeavy = (arrays[1].toFloat() + arrays[2].toFloat()) / 2;
            int count = m_totalCount - m_pkgInfors.size();
            max -= singleHeavy * count;
            min -= singleHeavy * count;
        }

        QString str = QString("结果： %1 门限[%2, %3] ").arg(weight).arg(min).arg(max);
        logs.append(str);

        if (weight <= max && weight >= min) {
            logs.append("PASS");
            showProcessLog(logs);
            return true;
        }
        logs.append("FAIL");
    }

    if (m_printAnyway) {
        showProcessLog("不满箱打印");
    }
    showProcessLog(logs);
    return false;
}


void productpacking::combineHeavy(QString name, float heavy, bool isProcuct)
{
    QString tips = QString("%1重量为：%2; ").arg(name).arg(heavy);
    tips += QString("裸机总重为：%1").arg(m_heavyNakeTotal);
    tips += QString("包装总重为：%1").arg(m_heavyTotal);
    showProcessLog(tips);
}

void productpacking::packing()
{
    if (m_errorCode != 0) return;

    showProcessLog("请放置包装箱到电子秤上面");

    float heavyLine = .0f;
    m_heavyTotal = .0f;
    m_heavyNakeTotal = .0f;
    m_pkgInfors.clear();
    m_snList.clear();
    m_printAnyway = false;
//    bool bRst = false;
//    for (int foo = 0; foo < 3; foo++) {
        msgBox("请放置包装箱到电子秤上面", 4);
        if (!getSingleWeight(heavyLine)) {
            m_errorCode = -3;
            m_errMsg = "获取包箱重量失败";
            return;
        }

//        if (heavyLine <= 0.0f) {
//            continue;
//        }

//        bRst = true;
//        break;
//    }

//    if (!bRst) {
//        m_errorCode = -3;
//        m_errMsg = "获取包箱重量失败";
//        return;
//    }

    showProcessLog("获取到包材初始重量为：" + QString::number(heavyLine, 'f', 4));
    m_heavyTotal += heavyLine;

    QString tips = "请扫描产品SN码";
    showProcessLog(tips);
    while(m_pkgInfors.size() < m_totalCount && !m_printAnyway) {
        if (m_layoutCount > 1 && (m_pkgInfors.size() > 0) && (m_pkgInfors.size() % m_singleCount == 0)) {
            msgBox("单层已满，请放置内衬泡棉后继续包装", 4);
            float heavyNc = .0f;
            if (!getSingleWeight(heavyNc)) {
                tips = "重量获取失败，请重新放置";
                showProcessLog(tips, -1);
                continue;
            }
            showProcessLog("获取到包材初始重量为：" + QString::number(heavyNc, 'f', 4));
            if (!heavyValidCheck(WTypeLineFoam, heavyNc)) {
                continue;
            }
            m_heavyTotal += heavyNc;
        }

        tips = "请扫码，扫完码放置于包装盒内 - 当前数量:" + QString::number(m_pkgInfors.size());
        QString sn = msgBox(tips);

        //////////////////////////////////////////////////////
        //// msgBox 在未满箱打印时会被关掉，这时候，需要在这里做循环的动作
        //// 此部分代码不可删除，除非逻辑修改
        if (m_printAnyway) {
            showProcessLog("执行未满箱打印");
            break;
        }
        //////////////////////////////////////////////////////

        showProcessLog("扫码到SN: " + sn);
        // SN比对
        if (!snValidCheck(sn)) {
            continue;
        }

        // XD01B的SN需要做特殊处理
        if (projectName().contains("XD01B")) {
            sn = sn.left(19);
            sn = sn.insert(15, '*');
            sn = sn.insert(15, '*');
        }

        setCurrentBarcode(sn);

        if (!m_mesObj.packageVerifySn(m_packBoxSN, sn)) {
            tips = sn + "校验失败，";
            tips += "请重新扫码或更换产品扫码";
            showProcessLog(tips, -1);
            continue;
        }
        showProcessLog(sn + "校验完成，请将产品放于电子秤上面");

        // 按照工程要求，取消此处的弹框，用超时读取的方式，读取稳定后重量值
        // msgBox("请将产品放置于电子秤", 4);
        float heavyTmp = .0f;
        QElapsedTimer timer;
        timer.start();
        bool bGetFlag = false;
        while (timer.elapsed() < 20000) {
            bGetFlag = getSingleWeight(heavyTmp);
            if (!bGetFlag) {
                tips = sn + " 获取重量失败，重试!!!";
                showProcessLog(tips);
                QThread::msleep(500);
                continue;
            }
            break;
        }

        if (!bGetFlag) {
            tips = sn + " 获取重量失败，请重新扫码进行操作!!!";
            showProcessLog(tips, -1);
            continue;
        }

        showProcessLog("获取到产品重量为：" + QString::number(heavyTmp, 'f', 4));

        if (!heavyValidCheck(WTypeSingleWeight, heavyTmp)) {
            continue;
        }

        if (m_StationName == "CL1007") {
            sn = sn.left(11);
            showProcessLog("镜头组件 取SN码前11位探测器编码: " + sn);
        }

        m_heavyNakeTotal += heavyTmp;
        m_heavyTotal += heavyTmp;
        combineHeavy(sn, heavyTmp, true);

        PackageInfor infor;
        infor.heavy = heavyTmp;
        infor.mobileSN = sn;
        m_pkgInfors.push_back(infor);
        m_snList.push_back(sn);

        QStringList list;
        list << sn << QString::number(heavyTmp, 'f', 4) << QString::number(m_heavyNakeTotal, 'f', 4) << QString::number(m_heavyTotal, 'f', 4) << "PASS";

        tableAddRowData(list);
        tableUpdateTestStatus(0);
    }

    showProcessLog("请将上盖泡棉到电子秤上面");
    msgBox("请将上盖泡棉到电子秤上面", 4);

    float heavyCover = .0f;
    if (!getSingleWeight(heavyCover)) {
        m_errorCode = -3;
        m_errMsg = "上盖泡棉称重失败";
        return;
    }
    showProcessLog("获取到上盖泡棉重量为：" + QString::number(heavyCover, 'f', 4));

    if (!heavyValidCheck(WTypeCoverFoam, heavyCover)) {
        m_errorCode = -3;
        m_errMsg = "上盖泡棉对比重量失败";
        return;
    }
    m_heavyTotal += heavyCover;

    if (!heavyValidCheck(WTypeFullWeight, m_heavyTotal)) {
        m_errorCode = -3;
        m_errMsg = "Packet总重对比重量失败";
        return;
    }
    if (!heavyValidCheck(WTypeNakedWeight, m_heavyNakeTotal)) {
        m_errorCode = -3;
        m_errMsg = "Nacked总重对比重量失败";
        return;
    }
}

void productpacking::afterPacking()
{
    if (m_errorCode != 0) return;

    // 扫码比对失败后可以执行3次打印比对...
#if 1
    bool brst = false;
    for (int foo = 0; foo < 3; foo++) {
        logAddNormalLog("执行打印");
        if (!print()) {
            QString tips = "打印失败，请检查环境重新执行打印，重试";
            resultShowProcess(tips);
            logAddFailLog(tips);
            continue;
        }

        logAddNormalLog("执行比对");
        if (!scanCompare()) {
            QString tips = "比对失败，请检查环境重新执行打印，重试";
            resultShowProcess(tips);
            logAddFailLog(tips);
            continue;
        }

        brst = true;
        break;
    }

    if (!brst) {
        m_errorCode = -4;
        m_errMsg = "执行SN比对失败...";
        return;
    }
#endif

    if (!bindPkgSnWithPSNs()) {
        return;
    }
}

void productpacking::deInit()
{
    m_elecScale.close();

    logAddNormalLog("执行数据保存");
    CsvLogger csvLogger;
    QString csvPath = mLogPath + QDate::currentDate().toString("yyyyMMdd") + ".csv";
    csvLogger.setCsvLogPath(csvPath);
    QStringList header;
    header << "开始时间" << "结束时间" << "测试结果" << "测试时间" << "包箱SN" << "总重量(kg)" << "裸机总重(kg)";
    QStringList list;
    list << "1" << QDateTime::currentDateTime().toString("yyyy/MM/dd hh:mm:ss") << ((m_errorCode == 0) ? "PASS" : "FAIL")
         << "0" << m_packBoxSN << QString::number(m_heavyTotal) << QString::number(m_heavyNakeTotal);

    for (int var = 0; var < m_pkgInfors.size(); ++var) {
        header << ("SN" + QString::number(var));
        header << ("单机重量(kg)");
        list << m_pkgInfors.at(var).mobileSN;
        list << QString::number(m_pkgInfors.at(var).heavy);
    }
    csvLogger.addCsvTitle(header);
    csvLogger.addCsvLog(list);

    QString fileName = mLogPath + (m_errorCode == 0 ? "PASS_" : "FAIL_") + m_packBoxSN + QTime::currentTime().toString("hhmmss");
    tableSaveCsv(fileName + ".csv");
    logSaveTxt(fileName + ".txt");

    if (m_errorCode == 0) {
        resultShowPass();
    } else {
        resultShowFail(m_errMsg);
    }

    msgBox("更换包装盒进行下一轮包装", 4);
    stopProcess();
}

bool productpacking::print()
{
    CMyPrinter printer;
    QString pName = printer.defalutPrinterName();
    logAddNormalLog("获取打印机名称: " + pName);
    if (pName.isEmpty()) {
        m_errorCode = -2;
        m_errMsg = "获取打印机名称失败!";
        return false;
    }

    QString tName = m_printTemplatePath;
    logAddNormalLog("获取模板文件本底路径: " + tName);
    if (!printer.load(pName, tName, 1, 1)) {
        m_errorCode = -3;
        m_errMsg = "打印机和打印模板加载失败";
        return false;
    }

    logAddNormalLog("数据替换");
    QStringList items = printer.items();
    qDebug() << items;
    foreach (QString a, items) {
        if (a.contains("ID010")) {
            QString woName = "ID010";
            QString woValue = m_workOrder;
            logAddNormalLog(woName + ":" + woValue);
            if (!printer.setItem(woName, woValue)) {
                m_errorCode = -4;
                m_errMsg = "W/O 模板字段替换失败";
                return false;
            }
        }

        if (a.contains("ID011")) {
            QString pkgName = "ID011";
            QString pkgNumber = m_workAACode;
            logAddNormalLog(pkgName + ":" + pkgNumber);
            if (!printer.setItem(pkgName, pkgNumber)) {
                m_errorCode = -4;
                m_errMsg = "P/N 模板字段替换失败";
                return false;
            }
        }

        if (a.contains("ID013")) {
            QString qName = "ID013";
            QString qValue = QString::number(m_pkgInfors.size()) + "PCS";
            logAddNormalLog(qName + ":" + qValue);
            if (!printer.setItem(qName, qValue)) {
                m_errorCode = -4;
                m_errMsg = "QTY 模板字段替换失败";
                return false;
            }
        }

        if (a.contains("ID012")) {
            QString cName = "ID012";
            QString cValue = m_snList.join(";");
            logAddNormalLog(cName + ":" + cValue);
            if (!printer.setItem(cName, cValue)) {
                m_errorCode = -4;
                m_errMsg = "二维码 模板字段替换失败";
                return false;
            }
        }

        if (a.contains("ID001")) {
            QString sDate = "ID001";
            QString sValue = QDate::currentDate().toString("yyyy.MM.dd");
            logAddNormalLog(sDate + ":" + sValue);
            if (!printer.setItem(sDate, sValue)) { // 后面改为从MES获取
                m_errorCode = -4;
                m_errMsg = "生产日期 模板字段替换失败";
                return false;
            }
        }

//        if (a.contains("ID016")) {
//            QString bName = "ID016";
//            QString bValue = "X800000000100" + QString("%1").arg(++mPackageSeq, 2, 10, QLatin1Char('0')); // 需要从MES获取
//            logAddNormalLog(bName + ":" + bValue);
//            if (!printer.setItem(bName, bValue)) {
//                m_errorCode = -4;
//                m_errMsg = "包装盒流水码 模板字段替换失败";
//                return false;
//            }
//        }
    }

    printer.print();

    return true;
}

bool productpacking::scanCompare()
{
    bool bRst = false;
    int times = 0;
    QString tips = "请扫码，以进行条码比对";
    do {
        QString str = msgBox(tips);
        QStringList list = str.split(";");
        if (list.size() != m_snList.size()) {
            tips = "条码数量不匹配，请重新扫码";
            continue;
        }

        QString pstr = m_snList.join(";");
        logAddNormalLog(str);
        logAddNormalLog(pstr);
        if (str.compare(pstr) != 0) {
            tips = "条码内容不匹配，请重新扫码";
            continue;
        }

        bRst = true;
        break;
    } while(times++ < 2);

    return bRst;
}

bool productpacking::bindPkgSnWithPSNs()
{
    if (m_pkgInfors.size() == m_totalCount || m_printAnyway) {
        logAddNormalLog("包装数据上传=>包箱SN: " + m_packBoxSN + "，箱内SN："  + m_snList.join(';'));
        if (!m_mesObj.packageBindSns(m_packBoxSN, m_snList)) {
            m_errMsg = QString("包箱%1绑定上传失败").arg(m_packBoxSN);
            logAddFailLog(m_errMsg);
            m_errorCode = -10;
        }
    } else {
        m_errMsg = "未知错误";
        logAddFailLog(m_errMsg);
        m_errorCode = -10;
    }
    return true;
}

void productpacking::showProcessLog(QString str, int flag)
{
    if (flag == -1) {
        logAddFailLog(str);
        showFail(str);
    } else {
        logAddNormalLog(str);
        showProcess(str);
    }
}


bool productpacking::getSingleWeight(float &weight)
{
    float heavy = .0f;
    if (!m_elecScale.getHeaveyQZ(heavy)) {
        return false;
    }
    weight = heavy - m_heavyTotal;
    return true;
}

bool productpacking::getTotalWeight(float &weight)
{
    if (!m_elecScale.getHeaveyQZ(weight)) {
        return false;
    }

    return true;
}


void productpacking::slotStopWorkBtnClicked()
{
    updateStatus(0);
    stopProcess();
    hideMsgBox();
}

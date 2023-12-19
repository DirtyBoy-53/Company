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
        showProcess("��û���κ����룬��ִ�д�ӡ");
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
        //showFail("��ȡ����ʧ��");
        return;
    }

    QStringList header;
    header << "��Ʒ���к�" << "��������(kg)" << "�׶�������(��������,��λkg)" << "�׶�������(������,��λkg)" << "���";
    tableAddHeader(header);

    mLogPath.clear();
    mLogPath = logPath() + "/";
    mLogPath += QDate::currentDate().toString("hhmmss") + "/";
    QDir dir(mLogPath);
    if (!dir.exists()) {
        dir.mkpath(mLogPath);
    }

    if (!m_elecScale.getHandle()) {
        showFail("���� DataLinkTransport ���ʧ��");
        return;
    }

    if (!m_elecScale.open(m_elecSerial, 9600, false)) {
        showFail("�� ���ӳƿ��ƴ���ʧ��!");
        return;
    }
    m_elecScale.close();

    resultShowProcess("��ʼ��OK��������ʼ������ťִ�в���");
    BaseProduce::slotConnectBtnClicked();
}

bool productpacking::getParam()
{
    m_StationName = ConfigInfo::getInstance()->cacheInfo().sCodeId;

    m_mesObj.setUser(MesClient::instance()->sUser());


    ConfigInfo::getInstance()->getValueString(m_StationName, "���ӳӴ���", m_elecSerial);
    ConfigInfo::getInstance()->getValueString(m_StationName, "��װ��ӡģ��", m_printTemplatePath);
    ConfigInfo::getInstance()->getValueBoolean(m_StationName, "�Ƿ�����ģ��", m_onlineTemplate);
    logAddNormalLog("��ǰ��λ���ô�ӡģ���MES��ȡ��" + QString(m_onlineTemplate ? "True" : "False"));

    if (m_onlineTemplate) {
        m_printTemplatePath.clear();
        m_printTemplatePath = QCoreApplication::applicationDirPath() + "/";
        m_printTemplatePath += QTime::currentTime().toString("hhmmss") + ".btw";
        if (!m_mesObj.getPackagePrintFile(m_printTemplatePath)) {
            m_errMsg = "���߻�ȡ��ӡģ��ʧ�ܣ���ȷ�ϴ˹����Ƿ������ô�ӡģ���ļ������л�Ϊ���״�ӡģ���ȡ";
            logAddFailLog(m_errMsg);
            showFail(m_errMsg);
            return false;
        }
    }

    QFileInfo fInfor(m_printTemplatePath);
    if (!fInfor.exists()) {
        m_errMsg = "ģ���ļ�������";
        logAddFailLog(m_errMsg);
        showFail(m_errMsg);
        return false;
    }
    logAddNormalLog("��ǰ��λ���ô�ӡģ��Ϊ��" + m_printTemplatePath);

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
        m_errMsg = "��ǰΪ������ģʽ�����л�����ҳ��ģʽ";
        return;
    }

    ConfigInfo::getInstance()->getValueInt32(m_StationName, "��װ��Ʒ����", m_totalCount);
    ConfigInfo::getInstance()->getValueInt32(m_StationName, "��װ��Ʒ����", m_layoutCount);
    m_singleCount = m_totalCount / m_layoutCount;
    logAddNormalLog(QString("��ǰ��װ������%1����װ����Ϊ��%2���������װ��%3").arg(m_totalCount).arg(m_layoutCount).arg(m_singleCount));
    logAddNormalLog("��ǰ��λ���ô�ӡģ��Ϊ��" + m_printTemplatePath);
    CacheInfo cInfo = ConfigInfo::getInstance()->cacheInfo();
    m_workOrder = cInfo.orderId;

    QString data = msgBox("��ȷ�Ϲ����ţ�" + m_workOrder + "�Ƿ���ȷ!!!", MSGBOX_SELECT);
    if (data != "PASS") {
        m_errorCode = -4;
        m_errMsg = "��ǰ�����Ų��ԣ����ڲ˵������й����л�!";
        return;
    }

    logAddWarningLog("��ǰ�����ù����ţ�" + m_workOrder);

    if (!m_elecScale.open(m_elecSerial, 9600, false)) {
        m_errorCode = -2;
        m_errMsg = "�������ӳӴ���ʧ��";
        return;
    }
}


void productpacking::beforePacking()
{
    if (m_errorCode != 0) return;

    if (0 != m_mesObj.getOrderSns(m_workOrder, m_packBoxSN, true)) {
        m_errMsg = "��ȡ��������ʧ��: " + m_packBoxSN;
        logAddFailLog(m_errMsg);
        m_errorCode = -1;
        return;
    } else {
        showProcessLog("��ȡ������SNΪ: " + m_packBoxSN);
    }

    m_workAACode = m_mesObj.getMaterialAACode(m_workOrder);
    logAddWarningLog("��ǰ����AA�룺" + m_workAACode);
    //
    showProcessLog("�뱣�ֵ��ӳӿ��أ����������������");
    msgBox("�뱣�ֵ��ӳӿ��أ����������������", 4);
    m_elecScale.setQP();
    m_elecScale.setGL();
}

bool productpacking::snValidCheck(QString sn)
{
    int snNeedLen = snGetLength();
    QString snPrefix = snGetPrefix();
    if (snNeedLen > 0) {
        if (sn.length() != snNeedLen) {
            QString tips = "SN����ƥ���������SN����Ϊ" + QString::number(snNeedLen);
            tips += ", ������ɨ��������Ʒɨ��";
            showProcessLog(tips);
            return false;;
        }
        if (snPrefix.size() != 0) {
            if (sn.indexOf(snPrefix) != 0) {
                QString tips = "SNǰ׺ƥ���������SNǰ׺Ϊ" + snPrefix;
                tips += ", ������ɨ��������Ʒɨ��";
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
        logs.append("��̨��Ʒ,");
        ConfigInfo::getInstance()->getValueString(m_StationName, "��̨��Ʒ��������������", rules);
        break;
    case WTypeLineFoam:
        logs.append("�ڳ�����,");
        ConfigInfo::getInstance()->getValueString(m_StationName, "�ڳ�������������������", rules);
        break;
    case WTypeCoverFoam:
        logs.append("�ϸ�����,");
        ConfigInfo::getInstance()->getValueString(m_StationName, "���޸���������������", rules);
        break;
    case WTypeNakedWeight:
        logs.append("�������,");
        ConfigInfo::getInstance()->getValueString(m_StationName, "�������������������", rules);
        break;
    case WTypeFullWeight:
        logs.append("��װ����,");
        ConfigInfo::getInstance()->getValueString(m_StationName, "��װ����������������", rules);
        break;
    }
    QStringList rulesArray = rules.split(";");
    if (rulesArray.size() != 3) {
        showProcessLog("�����������ļ���ʽ���Ϸ�");
        return false;
    }

    QString rSwitch = rulesArray.at(0);
    // bool isCheck = ((rSwitch.toInt() == 1) ? true : false);
    if (rSwitch.indexOf("1") == -1) {
        showProcessLog("����Ϊ��ִ�м�⣬����true������Ϊ��" + rSwitch);
        return true;
    } else {
        float min = rulesArray[1].toFloat();
        float max = rulesArray[2].toFloat();
        if ((WTypeNakedWeight == wt) && m_printAnyway) {
            QString myRules;
            ConfigInfo::getInstance()->getValueString(m_StationName, "��̨��Ʒ��������������", myRules);
            QStringList arrays = myRules.split(";");
            if (arrays.size() != 3) {
                showProcessLog("Get�����������ļ���ʽ���Ϸ�");
                return false;
            }
            float singleHeavy = (arrays[1].toFloat() + arrays[2].toFloat()) / 2;
            int count = m_pkgInfors.size();
            min = count * singleHeavy - fabs(max - min) / 2;
            max = count * singleHeavy + fabs(max - min) / 2;
        }

        if ((WTypeFullWeight == wt) && m_printAnyway) {
            QString myRules;
            ConfigInfo::getInstance()->getValueString(m_StationName, "��̨��Ʒ��������������", myRules);
            QStringList arrays = myRules.split(";");
            if (arrays.size() != 3) {
                showProcessLog("Get�����������ļ���ʽ���Ϸ�");
                return false;
            }

            float singleHeavy = (arrays[1].toFloat() + arrays[2].toFloat()) / 2;
            int count = m_totalCount - m_pkgInfors.size();
            max -= singleHeavy * count;
            min -= singleHeavy * count;
        }

        QString str = QString("����� %1 ����[%2, %3] ").arg(weight).arg(min).arg(max);
        logs.append(str);

        if (weight <= max && weight >= min) {
            logs.append("PASS");
            showProcessLog(logs);
            return true;
        }
        logs.append("FAIL");
    }

    if (m_printAnyway) {
        showProcessLog("�������ӡ");
    }
    showProcessLog(logs);
    return false;
}


void productpacking::combineHeavy(QString name, float heavy, bool isProcuct)
{
    QString tips = QString("%1����Ϊ��%2; ").arg(name).arg(heavy);
    tips += QString("�������Ϊ��%1").arg(m_heavyNakeTotal);
    tips += QString("��װ����Ϊ��%1").arg(m_heavyTotal);
    showProcessLog(tips);
}

void productpacking::packing()
{
    if (m_errorCode != 0) return;

    showProcessLog("����ð�װ�䵽���ӳ�����");

    float heavyLine = .0f;
    m_heavyTotal = .0f;
    m_heavyNakeTotal = .0f;
    m_pkgInfors.clear();
    m_snList.clear();
    m_printAnyway = false;
//    bool bRst = false;
//    for (int foo = 0; foo < 3; foo++) {
        msgBox("����ð�װ�䵽���ӳ�����", 4);
        if (!getSingleWeight(heavyLine)) {
            m_errorCode = -3;
            m_errMsg = "��ȡ��������ʧ��";
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
//        m_errMsg = "��ȡ��������ʧ��";
//        return;
//    }

    showProcessLog("��ȡ�����ĳ�ʼ����Ϊ��" + QString::number(heavyLine, 'f', 4));
    m_heavyTotal += heavyLine;

    QString tips = "��ɨ���ƷSN��";
    showProcessLog(tips);
    while(m_pkgInfors.size() < m_totalCount && !m_printAnyway) {
        if (m_layoutCount > 1 && (m_pkgInfors.size() > 0) && (m_pkgInfors.size() % m_singleCount == 0)) {
            msgBox("����������������ڳ����޺������װ", 4);
            float heavyNc = .0f;
            if (!getSingleWeight(heavyNc)) {
                tips = "������ȡʧ�ܣ������·���";
                showProcessLog(tips, -1);
                continue;
            }
            showProcessLog("��ȡ�����ĳ�ʼ����Ϊ��" + QString::number(heavyNc, 'f', 4));
            if (!heavyValidCheck(WTypeLineFoam, heavyNc)) {
                continue;
            }
            m_heavyTotal += heavyNc;
        }

        tips = "��ɨ�룬ɨ��������ڰ�װ���� - ��ǰ����:" + QString::number(m_pkgInfors.size());
        QString sn = msgBox(tips);

        //////////////////////////////////////////////////////
        //// msgBox ��δ�����ӡʱ�ᱻ�ص�����ʱ����Ҫ��������ѭ���Ķ���
        //// �˲��ִ��벻��ɾ���������߼��޸�
        if (m_printAnyway) {
            showProcessLog("ִ��δ�����ӡ");
            break;
        }
        //////////////////////////////////////////////////////

        showProcessLog("ɨ�뵽SN: " + sn);
        // SN�ȶ�
        if (!snValidCheck(sn)) {
            continue;
        }

        // XD01B��SN��Ҫ�����⴦��
        if (projectName().contains("XD01B")) {
            sn = sn.left(19);
            sn = sn.insert(15, '*');
            sn = sn.insert(15, '*');
        }

        setCurrentBarcode(sn);

        if (!m_mesObj.packageVerifySn(m_packBoxSN, sn)) {
            tips = sn + "У��ʧ�ܣ�";
            tips += "������ɨ��������Ʒɨ��";
            showProcessLog(tips, -1);
            continue;
        }
        showProcessLog(sn + "У����ɣ��뽫��Ʒ���ڵ��ӳ�����");

        // ���չ���Ҫ��ȡ���˴��ĵ����ó�ʱ��ȡ�ķ�ʽ����ȡ�ȶ�������ֵ
        // msgBox("�뽫��Ʒ�����ڵ��ӳ�", 4);
        float heavyTmp = .0f;
        QElapsedTimer timer;
        timer.start();
        bool bGetFlag = false;
        while (timer.elapsed() < 20000) {
            bGetFlag = getSingleWeight(heavyTmp);
            if (!bGetFlag) {
                tips = sn + " ��ȡ����ʧ�ܣ�����!!!";
                showProcessLog(tips);
                QThread::msleep(500);
                continue;
            }
            break;
        }

        if (!bGetFlag) {
            tips = sn + " ��ȡ����ʧ�ܣ�������ɨ����в���!!!";
            showProcessLog(tips, -1);
            continue;
        }

        showProcessLog("��ȡ����Ʒ����Ϊ��" + QString::number(heavyTmp, 'f', 4));

        if (!heavyValidCheck(WTypeSingleWeight, heavyTmp)) {
            continue;
        }

        if (m_StationName == "CL1007") {
            sn = sn.left(11);
            showProcessLog("��ͷ��� ȡSN��ǰ11λ̽��������: " + sn);
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

    showProcessLog("�뽫�ϸ����޵����ӳ�����");
    msgBox("�뽫�ϸ����޵����ӳ�����", 4);

    float heavyCover = .0f;
    if (!getSingleWeight(heavyCover)) {
        m_errorCode = -3;
        m_errMsg = "�ϸ����޳���ʧ��";
        return;
    }
    showProcessLog("��ȡ���ϸ���������Ϊ��" + QString::number(heavyCover, 'f', 4));

    if (!heavyValidCheck(WTypeCoverFoam, heavyCover)) {
        m_errorCode = -3;
        m_errMsg = "�ϸ����޶Ա�����ʧ��";
        return;
    }
    m_heavyTotal += heavyCover;

    if (!heavyValidCheck(WTypeFullWeight, m_heavyTotal)) {
        m_errorCode = -3;
        m_errMsg = "Packet���ضԱ�����ʧ��";
        return;
    }
    if (!heavyValidCheck(WTypeNakedWeight, m_heavyNakeTotal)) {
        m_errorCode = -3;
        m_errMsg = "Nacked���ضԱ�����ʧ��";
        return;
    }
}

void productpacking::afterPacking()
{
    if (m_errorCode != 0) return;

    // ɨ��ȶ�ʧ�ܺ����ִ��3�δ�ӡ�ȶ�...
#if 1
    bool brst = false;
    for (int foo = 0; foo < 3; foo++) {
        logAddNormalLog("ִ�д�ӡ");
        if (!print()) {
            QString tips = "��ӡʧ�ܣ����黷������ִ�д�ӡ������";
            resultShowProcess(tips);
            logAddFailLog(tips);
            continue;
        }

        logAddNormalLog("ִ�бȶ�");
        if (!scanCompare()) {
            QString tips = "�ȶ�ʧ�ܣ����黷������ִ�д�ӡ������";
            resultShowProcess(tips);
            logAddFailLog(tips);
            continue;
        }

        brst = true;
        break;
    }

    if (!brst) {
        m_errorCode = -4;
        m_errMsg = "ִ��SN�ȶ�ʧ��...";
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

    logAddNormalLog("ִ�����ݱ���");
    CsvLogger csvLogger;
    QString csvPath = mLogPath + QDate::currentDate().toString("yyyyMMdd") + ".csv";
    csvLogger.setCsvLogPath(csvPath);
    QStringList header;
    header << "��ʼʱ��" << "����ʱ��" << "���Խ��" << "����ʱ��" << "����SN" << "������(kg)" << "�������(kg)";
    QStringList list;
    list << "1" << QDateTime::currentDateTime().toString("yyyy/MM/dd hh:mm:ss") << ((m_errorCode == 0) ? "PASS" : "FAIL")
         << "0" << m_packBoxSN << QString::number(m_heavyTotal) << QString::number(m_heavyNakeTotal);

    for (int var = 0; var < m_pkgInfors.size(); ++var) {
        header << ("SN" + QString::number(var));
        header << ("��������(kg)");
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

    msgBox("������װ�н�����һ�ְ�װ", 4);
    stopProcess();
}

bool productpacking::print()
{
    CMyPrinter printer;
    QString pName = printer.defalutPrinterName();
    logAddNormalLog("��ȡ��ӡ������: " + pName);
    if (pName.isEmpty()) {
        m_errorCode = -2;
        m_errMsg = "��ȡ��ӡ������ʧ��!";
        return false;
    }

    QString tName = m_printTemplatePath;
    logAddNormalLog("��ȡģ���ļ�����·��: " + tName);
    if (!printer.load(pName, tName, 1, 1)) {
        m_errorCode = -3;
        m_errMsg = "��ӡ���ʹ�ӡģ�����ʧ��";
        return false;
    }

    logAddNormalLog("�����滻");
    QStringList items = printer.items();
    qDebug() << items;
    foreach (QString a, items) {
        if (a.contains("ID010")) {
            QString woName = "ID010";
            QString woValue = m_workOrder;
            logAddNormalLog(woName + ":" + woValue);
            if (!printer.setItem(woName, woValue)) {
                m_errorCode = -4;
                m_errMsg = "W/O ģ���ֶ��滻ʧ��";
                return false;
            }
        }

        if (a.contains("ID011")) {
            QString pkgName = "ID011";
            QString pkgNumber = m_workAACode;
            logAddNormalLog(pkgName + ":" + pkgNumber);
            if (!printer.setItem(pkgName, pkgNumber)) {
                m_errorCode = -4;
                m_errMsg = "P/N ģ���ֶ��滻ʧ��";
                return false;
            }
        }

        if (a.contains("ID013")) {
            QString qName = "ID013";
            QString qValue = QString::number(m_pkgInfors.size()) + "PCS";
            logAddNormalLog(qName + ":" + qValue);
            if (!printer.setItem(qName, qValue)) {
                m_errorCode = -4;
                m_errMsg = "QTY ģ���ֶ��滻ʧ��";
                return false;
            }
        }

        if (a.contains("ID012")) {
            QString cName = "ID012";
            QString cValue = m_snList.join(";");
            logAddNormalLog(cName + ":" + cValue);
            if (!printer.setItem(cName, cValue)) {
                m_errorCode = -4;
                m_errMsg = "��ά�� ģ���ֶ��滻ʧ��";
                return false;
            }
        }

        if (a.contains("ID001")) {
            QString sDate = "ID001";
            QString sValue = QDate::currentDate().toString("yyyy.MM.dd");
            logAddNormalLog(sDate + ":" + sValue);
            if (!printer.setItem(sDate, sValue)) { // �����Ϊ��MES��ȡ
                m_errorCode = -4;
                m_errMsg = "�������� ģ���ֶ��滻ʧ��";
                return false;
            }
        }

//        if (a.contains("ID016")) {
//            QString bName = "ID016";
//            QString bValue = "X800000000100" + QString("%1").arg(++mPackageSeq, 2, 10, QLatin1Char('0')); // ��Ҫ��MES��ȡ
//            logAddNormalLog(bName + ":" + bValue);
//            if (!printer.setItem(bName, bValue)) {
//                m_errorCode = -4;
//                m_errMsg = "��װ����ˮ�� ģ���ֶ��滻ʧ��";
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
    QString tips = "��ɨ�룬�Խ�������ȶ�";
    do {
        QString str = msgBox(tips);
        QStringList list = str.split(";");
        if (list.size() != m_snList.size()) {
            tips = "����������ƥ�䣬������ɨ��";
            continue;
        }

        QString pstr = m_snList.join(";");
        logAddNormalLog(str);
        logAddNormalLog(pstr);
        if (str.compare(pstr) != 0) {
            tips = "�������ݲ�ƥ�䣬������ɨ��";
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
        logAddNormalLog("��װ�����ϴ�=>����SN: " + m_packBoxSN + "������SN��"  + m_snList.join(';'));
        if (!m_mesObj.packageBindSns(m_packBoxSN, m_snList)) {
            m_errMsg = QString("����%1���ϴ�ʧ��").arg(m_packBoxSN);
            logAddFailLog(m_errMsg);
            m_errorCode = -10;
        }
    } else {
        m_errMsg = "δ֪����";
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

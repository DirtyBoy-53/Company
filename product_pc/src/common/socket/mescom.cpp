#include "mescom.h"
#include <util.h>
#include <QDebug>
#include <QCoreApplication>
#include <thread>

MesCom::MesCom()
{

}

void MesCom::setUser(QString sUser)
{
    m_user = sUser;
    m_client.setSUser(sUser);
}

int MesCom::login(QString user, QString passwd)
{
    return m_client.login(user, passwd);
}

int MesCom::getOrderSns(QString sOrderId, QString &result, bool is_package, bool isGetNew)
{
    return m_client.getOrderSns(sOrderId, result, is_package, isGetNew);
}

bool MesCom::enterProduce(QString sn, QString &sOutMsg, QString processCode)
{
    m_enterSuc = 0;
    m_mapData.clear();
    m_materData.clear();
    m_fileInfo.clear();
    for (int foo = 0; foo < 3; foo++) {
        if(0 == m_client.enterProduce(sn, m_mapData, m_materData, m_fileInfo, processCode)) {
            m_enterSuc = 1;
            m_mapData.remove("msg");
            return true;
        }

        if(m_mapData.contains("msg")) {
            sOutMsg = m_mapData["msg"].sValue;
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(300));
    }
    return false;
}

QString MesCom::getUser()
{
    return m_client.sUser();
}

QString MesCom::getPasswd()
{
    return m_client.sPass();
}

bool MesCom::outProduce(int result, QString &sOutMsg, QString sn)
{
    if(1 != m_enterSuc) {
        sOutMsg = "请先正常入站后，再出站";
        return false;
    }

    QMap<QString, MesCheckItem>::iterator iter;
    iter = m_mapData.begin();
    while(iter != m_mapData.end()) {
        if(iter.value().sResult != MESPASS && iter.value().sResult != MESFAILED) {
            iter.value().sResult = MESPASS;
        }
        iter++;
    }
    m_fileInfo.clear();
    QString sOut;
    //for (int foo = 0; foo < 3; foo++) {
        if(0 == m_client.outProduce(m_mapData, m_materData, result == 0 ? "PASS":"NG", sOut, sn)){
            sOutMsg = sOut;
            return true;
        }
        sOutMsg = sOut;
        //std::this_thread::sleep_for(std::chrono::milliseconds(300));
    //}
    return false;
}

void MesCom::setItemResult(MesCheckItem *pItem)
{
    if(!pItem) {
        return;
    }
    for(int j = 0; j < MESMAXCHECKNUM; j++) {
        if(pItem[j].sItem.size() <= 1) {
            break;
        }
        if(!m_mapData.contains(pItem[j].sItem)) {
            continue;
//            m_mapData.insert(pItem[j].sItem, *pItem);
        }
        m_mapData[pItem[j].sItem].sValue = pItem[j].sValue;
        m_mapData[pItem[j].sItem].sResult = pItem[j].sResult;
    }
    for(int j = 0; j < MESMAXCHECKNUM; j++) {
        if(pItem[j].sItem.size() <= 1) {
            break;
        }
        if(!m_materData.contains(pItem[j].sItem)) {
            continue;
//            m_mapData.insert(pItem[j].sItem, *pItem);
        }
        m_materData[pItem[j].sItem].sValue = pItem[j].sValue;
        qInfo()<<" "<<pItem[j].sItem << " "<<pItem[j].sValue;
    }
}

void MesCom::addItemResult(QString key, QString value, bool rst, QString err)
{
    MesCheckItem item;
    item.sItem = key;
    item.sValue = value;
    item.sResult = rst ? "PASS" : "NG";
    if(m_mapData.contains(key)) {
        m_mapData[key].sValue = value;
        m_mapData[key].sResult = rst ? "PASS" : "NG";
        m_mapData[key].sNote = err;
    }
    if(m_materData.contains(key)) {
        m_materData[key].sValue = value;
    }
}

void MesCom::addItemResultEx(MesCheckItem pItem)
{
    if (pItem.sItem.size() <= 1) {
        return;
    }
    if(!m_mapData.contains(pItem.sItem)) {
//            m_mapData.insert(pItem.sItem, pItem);
    } else {
        m_mapData[pItem.sItem]  = pItem;
    }
    if(m_materData.contains(pItem.sItem)) {
        m_materData[pItem.sItem].sValue = pItem.sValue;
    }
}

void MesCom::clearItem()
{
    m_mapData.clear();
}

bool MesCom::getBadPointInfo(QString sId, QStringList &outData, QString &sSensorSn)
{
    if(0 != m_client.getSensorData(sId, outData, sSensorSn)) {
        qInfo("get sensor id failed");
        return false;
    }

    return true;
}

bool MesCom::getVersionFile(QString savePath, QString type)
{
    QString token = m_fileInfo[type].sToken;
    qDebug() << "current token: " << token;
    if(token.size() <= 0) {
        return false;
    }
    QFileInfo fileInfo(savePath);
    if(fileInfo.isFile()) {
        qInfo()<<savePath<<" 已经存在";
        QFile::remove(savePath);
    }

    QByteArray outData;
    if(0 >= m_client.downloadData(token, outData, "product-file-version")) {
        qInfo("下载文件失败");
        return false;
    }
    qInfo("get size %d", outData.size());
    Util::createFileDir(fileInfo.absoluteDir().path());
    QFile file(savePath);
    file.open(QFile::WriteOnly);
    if(file.isOpen()) {
        file.write(outData);
        file.close();
    }else {
        qInfo("保存文件失败");
        return false;
    }
    return true;
}

bool MesCom::getUpgradeFileVersion(QString &version, QString type)
{
    if (m_fileInfo[type].sVersion.isEmpty()) {
        return false;
    }

    version = m_fileInfo[type].sVersion;
    return true;
}

QMap<QString, MesVersionFileInfo> MesCom::fileInfo() const
{
    return m_fileInfo;
}

bool MesCom::getPrintFile(QString &savePath)
{
    QString saveFile = QCoreApplication::applicationDirPath() + "/printTemp";
    Util::createFileDir(saveFile);

    QString sToken = m_client.productPrintTemplateUrl();
    if(sToken.size() <= 0) {
        return false;
    }
    QStringList list = sToken.split("/");
    if(list.size() <= 0) {
        qInfo("not fount \\ in %s", sToken.toStdString().data());
        return false;
    }
    saveFile = saveFile + "/" + list.at(list.size() - 1);
    QFileInfo fileInfo(saveFile);
    if(fileInfo.isFile()) {
        qInfo()<<saveFile<<" 已经存在";
        savePath = saveFile;
        return true;
    }
    qInfo()<<"保持打印模版 "<<saveFile;
    QByteArray outData;
    if(0 >=
            m_client.downloadData(sToken, outData, "xuanyuan")) {
        qInfo("下载文件失败");
        return false;
    }
    qInfo("get size %d", outData.size());

    QFile file(saveFile);
    file.open(QFile::WriteOnly);
    if(file.isOpen()) {
        file.write(outData);
        file.close();
        savePath = saveFile;
    }else {
        qInfo("保存文件失败");
        return false;
    }
    return true;
}

bool MesCom::getPackagePrintFile(QString &savePath)
{
    QString saveFile = QCoreApplication::applicationDirPath() + "/printTemp";
    Util::createFileDir(saveFile);

    QString sToken = m_client.packagePrintTemplateUrl();
    if(sToken.size() <= 0) {
        return false;
    }
    QStringList list = sToken.split("/");
    if(list.size() <= 0) {
        qInfo("not fount \\ in %s", sToken.toStdString().data());
        return false;
    }
    saveFile = saveFile + "/" + list.at(list.size() - 1);
    QFileInfo fileInfo(saveFile);
    if(fileInfo.isFile()) {
        qInfo()<<saveFile<<" 已经存在";
        return true;
    }
    qInfo()<<"保持打印模版 "<<saveFile;
    QByteArray outData;
    if(0 >=
            m_client.downloadData(sToken, outData, "xuanyuan")) {
        qInfo("下载文件失败");
        return false;
    }
    qInfo("get size %d", outData.size());

    QFile file(saveFile);
    file.open(QFile::WriteOnly);
    if(file.isOpen()) {
        file.write(outData);
        file.close();
        savePath = saveFile;
    }else {
        qInfo("保存文件失败");
        return false;
    }
    return true;
}

bool MesCom::getCheckInfo(QString itemName, MesCheckItem &item)
{
    bool bRet = false;
    if(m_mapData.contains(itemName)) {
        item = m_mapData[itemName];
        bRet = true;
    }
    return bRet;
}

bool MesCom::getMetrialList(QString itemId, QList<MaterialInfo> &list)
{
    bool bRet = false;
    if(0 == m_client.getSnList(itemId, list)) {
        bRet = true;
    }
    return bRet;
}

bool MesCom::getSensorSN(QString mobileSn, QString &sensorSn, QString keyWord)
{
    QList<MaterialInfo> list;
    if (getMetrialList(mobileSn, list)) {
        foreach(MaterialInfo i, list) {
            if (i.basMaterialName == keyWord) {
                sensorSn = i.mesBarCode;
                return true;
            }
        }
    }
    return false;
}

bool MesCom::getSnByMaterialSn(QString materialSn, QString &sn)
{
    if (m_client.getSnByMaterialSn(materialSn, sn) == 0) {
        return true;
    }
    return false;
}

bool MesCom::packageVerifySn(QString packageSn, QString sn)
{
    if (m_client.packageCheckSn(packageSn, sn) == 0) {
        return true;
    }

    return false;
}

bool MesCom::packageBindSns(QString packageSn, QStringList sns)
{
    if (m_client.packageBindSn(packageSn, sns) == 0) {
        return true;
    }

    return false;
}

QString MesCom::getMaterialAACode(QString order)
{
    return m_client.materialAACode(order);
}

QString MesCom::getOrderId()
{
    return m_client.orderCode();
}

QString MesCom::errString()
{
    return m_client.errString();
}

void MesCom::printMapData()
{
    QString log;
    QMap<QString, MesCheckItem>::iterator it = m_mapData.begin();
    for(; it != m_mapData.end(); ++it) {
        MesCheckItem item = it.value();
        log = QString("MES ITEM MAP>>> %1: item: %2, value: %3, result: %4").arg(it.key()).arg(item.sItem).arg(item.sValue).arg(item.sResult);
        qDebug() << log;
    }
}

void MesCom::printMasterData()
{
    QString log;
    QMap<QString, MesMaterItem>::iterator it = m_materData.begin();
    for(; it != m_materData.end(); ++it) {
        MesMaterItem item = it.value();
        log = QString("MES ITEM MASTER >>> %1: item: %2, value: %3").arg(it.key()).arg(item.sName).arg(item.sBarCode);
        qDebug() << log;
    }
}

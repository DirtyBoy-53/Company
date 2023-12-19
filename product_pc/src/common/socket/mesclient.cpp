#include "mesclient.h"
#include <qjsondocument.h>
#include <HttpClient.h>
#include <QFile>
#include <QJsonArray>
#include <configinfo.h>
#include <QNetworkInterface>
#include <util.h>

#define JSONTOSTR(json) QString(QJsonDocument(json).toJson(QJsonDocument::Compact)).simplified().toStdString()

static std::string g_serverIp = "192.168.110.23";
static int g_serverPort = 31453;

static std::string g_comand[20] = {
    "/api/mes-manufacture/plan/upperComputerOperate/login",
    "/api/mes-manufacture/minio/upload",
    "/api/mes-manufacture/minio/download",
    "/api/mes-manufacture/plan/upperComputerOperate/getMesPlanOrders",
    "/api/mes-manufacture/plan/upperComputerOperate/getAvailableProductBarCodes",
    "/api/mes-manufacture/plan/upperComputerOperate/inStack",
    "/api/mes-manufacture/plan/upperComputerOperate/outStack",
    "/api/mes-manufacture/plan/upperComputerOperate/getDataByMaterialSn",
    "/api/mes-manufacture/plan/upperComputerOperate/getBindedMaterialSnList",//获取绑定数据，或者通过绑定数据获取SN
    "/api/mes-manufacture/plan/upperComputerOperate/checkPackageCode",// 包装条码检验
    "/api/mes-manufacture/plan/upperComputerOperate/packageConfirm",// 包装结果上传绑定
    "/api/mes-manufacture/plan/mesPlanOrder/list2",// 获取整个的工单信息
};

MesClient::MesClient()
{
    m_sUser = "";
    m_sPass = "";
    g_serverIp = ConfigInfo::getInstance()->getValueString("MES服务IP").toStdString();
    g_serverPort = ConfigInfo::getInstance()->getValueString("MES服务端口").toInt();

    m_mapSnMesOrderId.clear();
}

int MesClient::login(QString &sUserName, QString &sPass)
{
    g_serverIp = ConfigInfo::getInstance()->getValueString("MES服务IP").toStdString();
    g_serverPort = ConfigInfo::getInstance()->getValueString("MES服务端口").toInt();

    HttpClient client(g_serverIp, g_serverPort);
    QJsonObject json;
    json.insert("userName", sUserName);
    json.insert("password", sPass);
    std::string sOut;
    m_sUser = "";
    m_sPass = "";
    int ret = client.post(g_comand[0], JSONTOSTR(json), sOut);
    if(ret != RET_OK) {
        return -2;
    }
    m_sUser = sUserName;
    m_sPass = sPass;
    qInfo("this %x recv %s user %s",this, sOut.data(), m_sUser.toStdString());

    QJsonParseError jsonError;
    QJsonDocument document = QJsonDocument::fromJson(sOut.data(), &jsonError);
    if(document.isNull() || !document.isObject() || jsonError.error != QJsonParseError::NoError) {
        return -1;
    }

    QJsonObject object = document.object();
    int iRet = -1;
    if(object.contains("code")) {
        if( 0 == object.value("code").toInt()) {
            iRet = 0;
        }
    }
    if(iRet >= 0 && object.contains("data")) {
        int role = object.value("data").toInt();
        if(role == 1) {
            iRet = 0;
        }else if(role == 2) {
            iRet = 1;
        } else {
            iRet = 2;
        }
        m_sUser = sUserName;
        m_sPass = sPass;
    }
    return iRet;
}

int MesClient::getStardData(QList<ResultInfo> &listData)
{
    return 0;
}

int MesClient::enterProduce(QString sn,
                            QMap<QString, MesCheckItem> &mapData,
                            QMap<QString, MesMaterItem> & materItem,
                            QMap<QString, MesVersionFileInfo> & fileInfo,
                            QString processCode)
{
    m_materialAACode.clear();
    g_serverIp = ConfigInfo::getInstance()->getValueString("MES服务IP").toStdString();
    g_serverPort = ConfigInfo::getInstance()->getValueString("MES服务端口").toInt();

    HttpClient client(g_serverIp, g_serverPort);
    std::string sOut;
    CacheInfo info = ConfigInfo::getInstance()->cacheInfo();
    QJsonObject json;
    m_deviceCode = Util::getHostMacAddress();
    json.insert("deviceCode", m_deviceCode);
    if (processCode.isEmpty()) {
        json.insert("processCode", info.sCodeId);
    } else {
        json.insert("processCode", processCode);
    }

    json.insert("barCode", sn);
    json.insert("userName", m_sUser);
    int ret = client.post(g_comand[5], JSONTOSTR(json), sOut);
    if(ret != RET_OK) {
        return -2;
    }
    qInfo("this %x recv %s user name %s ",this, sOut.data(), m_sUser.toStdString().data());
    QJsonParseError jsonError;
    QJsonDocument document = QJsonDocument::fromJson(sOut.data(), &jsonError);
    if(document.isNull() || !document.isObject() || jsonError.error != QJsonParseError::NoError) {
        return -1;
    }

    QJsonObject object = document.object();
    int iRet = -1;
    if(object.contains("code")) {
        if( 0 == object.value("code").toInt()) {
            iRet = 0;
            if(object.value("data").isObject()) {
                QJsonObject dataObj = object.value("data").toObject();
                info.finishedQuantity = dataObj.value("finishedQuantity").toInt();
                info.unfinishedQuantity = dataObj.value("unfinishedQuantity").toInt();

                QJsonArray arr = dataObj.value("checkItemConfigList").toArray();
                if(dataObj.contains("mesWorkOrderId")) {
                    m_workOrderId = dataObj.value("mesWorkOrderId").toString();
                    m_mapSnMesOrderId[sn] = m_workOrderId;
                }
                if(dataObj.contains("mesWorkOrder")) {
                    info.orderId = dataObj.value("mesWorkOrder").toString();
                }
                ConfigInfo::getInstance()->setCacheInfo(info);

                for(int i = 0; i < arr.size(); i ++) {
                    if(arr[i].isObject()){
                        QJsonObject obj = arr[i].toObject();
                        MesCheckItem item;
                        item.sItem = obj.value("checkItemCode").toString();
                        item.sValue = obj.value("standardValue").toString();
                        if(obj.contains("minValue")) {
                            item.sMin = obj.value("minValue").toString();
                        }
                        if(obj.contains("maxValue")) {
                            item.sMax = obj.value("maxValue").toString();
                        }
                        mapData[item.sItem] = item;
                    }
                }

                QJsonArray arrMater = dataObj.value("materialList").toArray();
                for(int i = 0; i < arrMater.size(); i ++) {
                    if(arrMater[i].isObject()){
                        QJsonObject obj = arrMater[i].toObject();
                        MesMaterItem item;
                        item.sId = obj.value("materialId").toString();
                        item.sName = obj.value("materialName").toString();

                        materItem[item.sName] = item;
                    }
                }
                QJsonArray arrFile = dataObj.value("barCodeDtoList").toArray();
                for(int i = 0; i < arrMater.size(); i ++) {
                    if(arrFile[i].isObject()){
                        QJsonObject obj = arrFile[i].toObject();
                        MesMaterItem item;
                        item.sBarCode = obj.value("barCode").toString();
                        item.sFileUrl = obj.value("fileUrl").toString();

                        materItem[item.sBarCode] = item;
                    }
                }
//                if(dataObj.contains("basProductFileName")) {
//                    fileInfo.sName = dataObj.value("basProductFileName").toString();
//                }
//                if(dataObj.contains("basProductFileVersion")) {
//                    fileInfo.sVersion = dataObj.value("basProductFileVersion").toString();
//                }
//                if(dataObj.contains("basProductFilePath")) {
//                    fileInfo.sToken = dataObj.value("basProductFilePath").toString();
//                }
                if (dataObj.contains("asicDto")) {
                    MesVersionFileInfo mesFile;
                    QJsonObject asicDtoObj = dataObj.value("asicDto").toObject();
                    if (asicDtoObj.contains("fileName")) {
                        mesFile.sName = asicDtoObj.value("fileName").toString();
                    }
                    if (asicDtoObj.contains("fileVersion")) {
                        mesFile.sVersion = asicDtoObj.value("fileVersion").toString();
                    }
                    if (asicDtoObj.contains("filePath")) {
                        mesFile.sToken = asicDtoObj.value("filePath").toString();
                    }
                    fileInfo["asic"] = mesFile;
                }
                if (dataObj.contains("mcuDto")) {
                    MesVersionFileInfo mesFile;
                    QJsonObject asicDtoObj = dataObj.value("mcuDto").toObject();
                    if (asicDtoObj.contains("fileName")) {
                        mesFile.sName = asicDtoObj.value("fileName").toString();
                    }
                    if (asicDtoObj.contains("fileVersion")) {
                        mesFile.sVersion = asicDtoObj.value("fileVersion").toString();
                    }
                    if (asicDtoObj.contains("filePath")) {
                        mesFile.sToken = asicDtoObj.value("filePath").toString();
                    }
                    fileInfo["mcu"] = mesFile;
                }
                if (dataObj.contains("armDto")) {
                    MesVersionFileInfo mesFile;
                    QJsonObject asicDtoObj = dataObj.value("armDto").toObject();
                    if (asicDtoObj.contains("fileName")) {
                        mesFile.sName = asicDtoObj.value("fileName").toString();
                    }
                    if (asicDtoObj.contains("fileVersion")) {
                        mesFile.sVersion = asicDtoObj.value("fileVersion").toString();
                    }
                    if (asicDtoObj.contains("filePath")) {
                        mesFile.sToken = asicDtoObj.value("filePath").toString();
                    }
                    fileInfo["arm"] = mesFile;
                }
                if(dataObj.contains("mesPlanOrderCode")) {
                    m_orderCode = dataObj.value("mesPlanOrderCode").toString();
                    if(info.orderId.size() <= 0) {
                        info.orderId = m_orderCode;
                        ConfigInfo::getInstance()->setCacheInfo(info);
                    }
                }
                if(dataObj.contains("productPrintTemplateUrl")) {
                    m_productPrintTemplateUrl = dataObj.value("productPrintTemplateUrl").toString();
                }
                if(dataObj.contains("packagePrintTemplateUrl")) {
                    m_packagePrintTemplateUrl = dataObj.value("packagePrintTemplateUrl").toString();
                }
                if(dataObj.contains("basProductCode")) {
                    m_materialAACode = dataObj.value("basProductCode").toString();
                }
            }
        }else {

        }
        m_fileInfo = fileInfo;
    }

    if(object.contains("msg")) {
        MesCheckItem item;
        item.sItem = "msg";
        item.sValue = object.value("msg").toString();
        mapData[item.sItem] = item;
    }
    return iRet;
}

int MesClient::outProduce(QMap<QString, MesCheckItem> &mapData, QMap<QString, MesMaterItem> &materItem, QString sResult, QString &sMsgOut, QString sn)
{
    g_serverIp = ConfigInfo::getInstance()->getValueString("MES服务IP").toStdString();
    g_serverPort = ConfigInfo::getInstance()->getValueString("MES服务端口").toInt();

    HttpClient client(g_serverIp, g_serverPort);
    std::string sOut;
    CacheInfo info = ConfigInfo::getInstance()->cacheInfo();
    QJsonObject json;
    json.insert("deviceCode", m_deviceCode);
    if (!sn.isEmpty()) {
        json.insert("mesWorkOrderId", m_mapSnMesOrderId[sn]);
    } else {
        json.insert("mesWorkOrderId", m_workOrderId);
    }
    json.insert("userName", m_sUser);
    json.insert("outStackResult", sResult);
    QJsonArray arr;
    QMap<QString, MesCheckItem>::iterator iter;
    for(iter = mapData.begin(); iter != mapData.end(); iter++) {
        QJsonObject obj;
        obj.insert("actualValue", iter.value().sValue);
        obj.insert("checkItemCode", iter.value().sItem);
        obj.insert("checkResult", iter.value().sResult);
        obj.insert("remark", iter.value().sNote);
        arr.append(obj);
    }
    json.insert("checkItemList", arr);


    QJsonArray arrMa;
    QMap<QString, MesMaterItem>::iterator iterMa;
    for(iterMa = materItem.begin(); iterMa != materItem.end(); iterMa++) {
        QJsonObject obj;
        if(iterMa.value().sId.length() > 0) {
            obj.insert("materialId", iterMa.value().sId);
            obj.insert("barCode", iterMa.value().sValue);
            arrMa.append(obj);
        }
    }

    json.insert("materialList", arrMa);
    int ret = client.post(g_comand[6], JSONTOSTR(json), sOut);
    if(ret != RET_OK) {
        return -2;
    }
    qInfo("this %x recv %s user name %s ",this, sOut.data(), m_sUser.toStdString().data());
    QJsonParseError jsonError;
    QJsonDocument document = QJsonDocument::fromJson(sOut.data(), &jsonError);
    if(document.isNull() || !document.isObject() || jsonError.error != QJsonParseError::NoError) {
        return -1;
    }
    QJsonObject object = document.object();
    int iRet = -1;
    if(object.contains("code")) {
        if( 0 == object.value("code").toInt()) {
            iRet = 0;
            m_mapSnMesOrderId.remove(sn);
        }else {
            sMsgOut = object.value("msg").toString();
        }
    }
    return iRet;
}

int MesClient::uploadData(QString &sOutToken, QByteArray &array)
{
    g_serverIp = ConfigInfo::getInstance()->getValueString("MES服务IP").toStdString();
    g_serverPort = ConfigInfo::getInstance()->getValueString("MES服务端口").toInt();

    HttpClient client(g_serverIp, g_serverPort);
    std::string sOut;
    int ret = client.upload(g_comand[1], "test.bin",  (uint8_t *)array.data(), array.size(), sOut);
    if(ret != RET_OK) {
        return -2;
    }
    qInfo("recv %s",sOut.data());
    QJsonParseError jsonError;
    QJsonDocument document = QJsonDocument::fromJson(sOut.data(), &jsonError);
    if(document.isNull() || !document.isObject() || jsonError.error != QJsonParseError::NoError) {
        return -1;
    }

    QJsonObject object = document.object();
    int iRet = -1;
    if(object.contains("code")) {
        if( 0 == object.value("code").toInt()) {
            if(object.value("data").isObject()) {
                sOutToken = object.value("data").toObject().value("url").toString();
                iRet = 0;
            }
        }
    }
    return iRet;
}

int MesClient::downloadData(QString &sToken, QByteArray &array, QString sKeyName)
{
    g_serverIp = ConfigInfo::getInstance()->getValueString("MES服务IP").toStdString();
    g_serverPort = ConfigInfo::getInstance()->getValueString("MES服务端口").toInt();

    HttpClient client(g_serverIp, g_serverPort);
    std::string sOut;
    uint8_t * pData = nullptr;
    int rLen = client.download(g_comand[2], sToken.toStdString(),  &pData, sKeyName.toStdString());
    if(rLen <= 0 ) {
        qInfo("download error ");
        return -2;
    }

    if(pData) {
        array = QByteArray((char *)pData, rLen);
        free(pData);
    }
    return rLen;
}

QString MesClient::sUser() const
{
    return m_sUser;
}

void MesClient::setSUser(const QString &newSUser)
{
    m_sUser = newSUser;
}

QString MesClient::sPass() const
{
    return m_sPass;
}

void MesClient::setSPass(const QString &newSPass)
{
    m_sPass = newSPass;
}

int MesClient::getOrderIds( QList<ResultInfo> &result)
{
    g_serverIp = ConfigInfo::getInstance()->getValueString("MES服务IP").toStdString();
    g_serverPort = ConfigInfo::getInstance()->getValueString("MES服务端口").toInt();

    HttpClient client(g_serverIp, g_serverPort);
    std::string sOut;
    CacheInfo info = ConfigInfo::getInstance()->cacheInfo();
    std::map<std::string,std::string>  param;
    param["basProcessCode"] = info.sCodeId.toStdString() ;
    int ret = client.get(g_comand[3], param,  sOut);
    if(ret != RET_OK) {
        return -2;
    }
    qInfo("recv %s",sOut.data());
    QJsonParseError jsonError;
    QJsonDocument document = QJsonDocument::fromJson(sOut.data(), &jsonError);
    if(document.isNull() || !document.isObject() || jsonError.error != QJsonParseError::NoError) {
        return -1;
    }

    QJsonObject object = document.object();
    int iRet = -1;
    if(object.contains("code")) {
        if( 0 == object.value("code").toInt()) {
            if(object.value("data").isArray()) {
                QJsonArray arr = object.value("data").toArray();
                for(int i = 0;i < arr.size(); i ++) {
                    if(arr.at(i).isObject()) {
                        if(arr.at(i).toObject().contains("mesPlanOrderCode")) {
                            ResultInfo r;
                            r.key = arr.at(i).toObject().value("mesPlanOrderCode").toString();
                            result.push_back(r);
                        }
                    }

                }
                iRet = 0;
            }
        }
    }

    return iRet;
}

int MesClient::getOrderSns(QString sOrderId, QString &result, bool isPackage, bool isCreateNewEveryTime)
{
    g_serverIp = ConfigInfo::getInstance()->getValueString("MES服务IP").toStdString();
    g_serverPort = ConfigInfo::getInstance()->getValueString("MES服务端口").toInt();

    HttpClient client(g_serverIp, g_serverPort);
    std::string sOut;
    std::map<std::string,std::string>  param;
    param["isPackage"] = isPackage ? "true" : "false";
    param["isCreateNewEveryTime"] = isCreateNewEveryTime ? "true" : "false";
    param["mesPlanOrderCode"] = sOrderId.toStdString() ;
    int ret = client.get(g_comand[4], param,  sOut);
    if(ret != RET_OK) {
        return -2;
    }
    qInfo("recv %s",sOut.data());
    QJsonParseError jsonError;
    QJsonDocument document = QJsonDocument::fromJson(sOut.data(), &jsonError);
    if(document.isNull() || !document.isObject() || jsonError.error != QJsonParseError::NoError) {
        return -1;
    }

    QJsonObject object = document.object();
    int iRet = -1;
    if(object.contains("code")) {
        if( 0 == object.value("code").toInt()) {
            if(object.value("data").isArray()) {
                QJsonArray arr = object.value("data").toArray();
                for(int i = 0;i < arr.size(); i ++) {
                    if(arr.at(i).isString()) {
                        result = arr.at(i).toString();
                        iRet = 0;
                        break;
                    }
                }
            }
        }else {
            result = object.value("msg").toString();
        }
    }

    return iRet;
}

int MesClient::getSensorData(QString sn, QStringList &result, QString &sSenserSn)
{
    g_serverIp = ConfigInfo::getInstance()->getValueString("MES服务IP").toStdString();
    g_serverPort = ConfigInfo::getInstance()->getValueString("MES服务端口").toInt();

    HttpClient client(g_serverIp, g_serverPort);
    std::string sOut;
    std::map<std::string,std::string>  param;
    param["materialSn"] = sn.toStdString();
    int ret = client.get(g_comand[7], param,  sOut);
    if(ret != RET_OK) {
        return -2;
    }
    qInfo("recv %s",sOut.data());
    QJsonParseError jsonError;
    QJsonDocument document = QJsonDocument::fromJson(sOut.data(), &jsonError);
    if(document.isNull() || !document.isObject() || jsonError.error != QJsonParseError::NoError) {
        return -1;
    }

    QJsonObject object = document.object();
    int iRet = -1;
    if(object.contains("code")) {
        if( 0 == object.value("code").toInt()) {
            if(object.value("data").isArray()) {
                QJsonArray arr = object.value("data").toArray();
                if(arr.size() <= 0 ) {
                    return iRet;
                }
                QJsonObject arrD = arr[0].toObject();
                if(arrD.contains("contentList")) {
                    QJsonArray dArr = arrD.value("contentList").toArray();
                    result.clear();
                    for(int i = 0; i < dArr.size(); i++) {
                        result<<dArr[i].toString();
                    }
                }
                if(arrD.contains("materialSn")) {
                    sSenserSn = arrD.value("materialSn").toString();
                }
                iRet = 0;
            }
        }
    }

    return iRet;
}

QString MesClient::productPrintTemplateUrl() const
{
    return m_productPrintTemplateUrl;
}

QString MesClient::packagePrintTemplateUrl() const
{
    return m_packagePrintTemplateUrl;
}

QString MesClient::orderCode() const
{
    return m_orderCode;
}

QMap<QString, MesVersionFileInfo> MesClient::fileInfo() const
{
    return m_fileInfo;
}

int MesClient::getSnList(QString sItem, QList<MaterialInfo> &list)
{

    g_serverIp = ConfigInfo::getInstance()->getValueString("MES服务IP").toStdString();
    g_serverPort = ConfigInfo::getInstance()->getValueString("MES服务端口").toInt();

    HttpClient client(g_serverIp, g_serverPort);
    std::string sOut;
    std::map<std::string,std::string>  param;
    param["materialSn"] = sItem.toStdString();
    int ret = client.get(g_comand[8], param,  sOut);
    if(ret != RET_OK) {
        return -2;
    }
    qInfo("recv %s",sOut.data());
    QJsonParseError jsonError;
    QJsonDocument document = QJsonDocument::fromJson(sOut.data(), &jsonError);
    if(document.isNull() || !document.isObject() || jsonError.error != QJsonParseError::NoError) {
        return -1;
    }

    QJsonObject object = document.object();
    int iRet = -1;
    if(object.contains("code")) {
        if( 0 == object.value("code").toInt()) {
            if (!object.contains("data")) {
                qDebug() << "不包含data字段，FAIL";
                return iRet;
            }
            QJsonObject objectX = object.value("data").toObject();
            if (!objectX.contains("materialSnList")) {
                qDebug() << "不包含materialSnList字段，FAIL";
                return iRet;
            }
            if(objectX.value("materialSnList").isArray()) {
                qDebug() << "materialSnList is array..";
                QJsonArray arr = objectX.value("materialSnList").toArray();
                if(arr.size() <= 0 ) {
                    return iRet;
                }

                for(int i = 0; i < arr.size(); i ++) {
                    MaterialInfo info;
                    QJsonObject arrD = arr[i].toObject();

                    for(int i = 0; i < arr.size(); i ++) {
                        if(arrD.contains("mesBarCode")) {
                            info.mesBarCode = arrD.value("mesBarCode").toString();
                        }
                        if(arrD.contains("basMaterialName")) {
                            info.basMaterialName = arrD.value("basMaterialName").toString();
                        }
                        if(arrD.contains("basMaterialCode")) {
                            info.basMaterialCode = arrD.value("basMaterialCode").toString();
                        }
                        if(arrD.contains("basMaterialSprc")) {
                            info.basMaterialSprc = arrD.value("basMaterialSprc").toString();
                        }
                    }
                    list.append(info);
                }
                iRet = 0;
            }
        }
    }

    return iRet;
}

int MesClient::getSnByMaterialSn(QString materialSn, QString &sn)
{
    g_serverIp = ConfigInfo::getInstance()->getValueString("MES服务IP").toStdString();
    g_serverPort = ConfigInfo::getInstance()->getValueString("MES服务端口").toInt();

    HttpClient client(g_serverIp, g_serverPort);
    std::string sOut;
    std::map<std::string,std::string>  param;
    param["materialSn"] = materialSn.toStdString();
    int ret = client.get(g_comand[8], param,  sOut);
    if(ret != RET_OK) {
        return -2;
    }
    qInfo("recv %s",sOut.data());
    QJsonParseError jsonError;
    QJsonDocument document = QJsonDocument::fromJson(sOut.data(), &jsonError);
    if(document.isNull() || !document.isObject() || jsonError.error != QJsonParseError::NoError) {
        return -1;
    }

    QJsonObject object = document.object();
    int iRet = -1;
    if(object.contains("code")) {
        if( 0 == object.value("code").toInt()) {
            if (!object.contains("data")) {
                qDebug() << "不包含data字段，FAIL";
                return iRet;
            }
            QJsonObject objectX = object.value("data").toObject();
            if (!objectX.contains("productBarCode")) {
                qDebug() << "不包含productBarCode字段，FAIL";
                return iRet;
            }

            sn = objectX["productBarCode"].toString();
            return 0;
        }
    }

    return iRet;
}

int MesClient::getOrderInfor(QString code)
{
    m_orderInfor.clear();
    g_serverIp = ConfigInfo::getInstance()->getValueString("MES服务IP").toStdString();
    g_serverPort = ConfigInfo::getInstance()->getValueString("MES服务端口").toInt();

    HttpClient client(g_serverIp, g_serverPort);
    std::string sOut;
    std::map<std::string,std::string>  param;
    param["code"] = code.toStdString();
    int ret = client.get(g_comand[11], param, sOut);
    if(ret != RET_OK) {
        return -2;
    }

    qInfo("this %x recv %s user name %s ",this, sOut.data(), m_sUser.toStdString().data());
    QJsonParseError jsonError;
    QJsonDocument document = QJsonDocument::fromJson(sOut.data(), &jsonError);
    if(document.isNull() || !document.isObject() || jsonError.error != QJsonParseError::NoError) {
        return -1;
    }

    QJsonObject object = document.object();
    int iRet = -1;
    if(object.contains("code")) {
        if( 0 == object.value("code").toInt()) {
            iRet = 0;
            if(object.value("data").isObject()) {
                QJsonObject dataObj = object.value("data").toObject();
                if (!dataObj.contains("total")) {
                    return -2;
                }

                int total = dataObj.value("total").toInt();
                if (total < 1) {
                    return -2;
                }
                if (dataObj.value("records").isArray()) {
                    QJsonArray records = dataObj.value("records").toArray();
                    qDebug() << records.size();
                    if (records.size() != total) {
                        qDebug() << "返回数组长度异常";
                        return -2;
                    }
                    QJsonObject objX = records[0].toObject();
                    if (objX.contains("code")) {
                        QString qCode = objX.value("code").toString();
                        qDebug() << qCode;
                        if (qCode != code) {
                            return -2;
                        }
                        m_orderInfor.orderId = qCode;
                    }

                    if (objX.contains("basProductCode")) {
                        m_orderInfor.productCode = objX.value("basProductCode").toString();
                    }

                    if (objX.contains("basProductName")) {
                        m_orderInfor.productName = objX.value("basProductName").toString();
                    }

                    return 0;
                }
            }
        }

    }


    return iRet;
}

int MesClient::packageCheckSn(QString packageSn, QString productSn)
{
    g_serverIp = ConfigInfo::getInstance()->getValueString("MES服务IP").toStdString();
    g_serverPort = ConfigInfo::getInstance()->getValueString("MES服务端口").toInt();

    HttpClient client(g_serverIp, g_serverPort);
    std::string sOut;
    std::map<std::string,std::string>  param;
    param["packageBarCode"] = packageSn.toStdString();
    param["productBarCode"] = productSn.toStdString();
    int ret = client.get(g_comand[9], param,  sOut);
    if(ret != RET_OK) {
        return -2;
    }
    qInfo("recv %s",sOut.data());
    QJsonParseError jsonError;
    QJsonDocument document = QJsonDocument::fromJson(sOut.data(), &jsonError);
    if(document.isNull() || !document.isObject() || jsonError.error != QJsonParseError::NoError) {
        return -1;
    }

    QJsonObject object = document.object();
    int iRet = -1;
    if(object.contains("code")) {
        if( 0 != object.value("code").toInt()) {
            if (object.contains("msg")) {
                m_errString = object.value("msg").toString();
                return iRet;
            }
        } else {
            if (object.contains("success")) {
                if (object.value("success").toBool()) {
                    return 0;
                }
            }
        }
    }

    return iRet;
}

int MesClient::packageBindSn(QString packageSn, QStringList productSns)
{
    g_serverIp = ConfigInfo::getInstance()->getValueString("MES服务IP").toStdString();
    g_serverPort = ConfigInfo::getInstance()->getValueString("MES服务端口").toInt();

    HttpClient client(g_serverIp, g_serverPort);
    std::string sOut;
    QJsonObject json;
    m_deviceCode = Util::getHostMacAddress();
    json.insert("deviceCode", m_deviceCode);
    json.insert("packageBarcode", packageSn);

    QJsonArray array;
    foreach (QString a, productSns) {
        array.append(a);
    }
    json.insert("productBarCodes", array);
    json.insert("userName", m_sUser);
    int ret = client.post(g_comand[10], JSONTOSTR(json), sOut);
    if(ret != RET_OK) {
        return -2;
    }
    qInfo("this %x recv %s user name %s ",this, sOut.data(), m_sUser.toStdString().data());
    QJsonParseError jsonError;
    QJsonDocument document = QJsonDocument::fromJson(sOut.data(), &jsonError);
    if(document.isNull() || !document.isObject() || jsonError.error != QJsonParseError::NoError) {
        return -1;
    }

    QJsonObject object = document.object();
    int iRet = -1;
    if(object.contains("code")) {
        if( 0 != object.value("code").toInt()) {
            if (object.contains("msg")) {
                m_errString = object.value("msg").toString();
                return iRet;
            }
        } else {
            if (object.contains("success")) {
                if (object.value("success").toBool()) {
                    return 0;
                }
            }
        }
    }
    return iRet;
}

QString MesClient::errString()
{
    return m_errString;
}

QString MesClient::materialAACode(QString order)
{
    if (0 != getOrderInfor(order)) {
        return "";
    }

    return m_orderInfor.productCode;
}

QString MesClient::orderId() const
{
    return m_workOrderId;
}



#include "initeractionbyhttp.h"
#include <HttpClient.h>
#include <QJsonDocument>
#include <memcache.h>
#include <QDateTime>
#include <QDebug>
#include <QJsonArray>

#define HTTPERRORNO -1000
#define HTTPPORT 80
#define JSONTOSTR(json) QString(QJsonDocument(json).toJson(QJsonDocument::Compact)).simplified().toStdString()

#define CHECKNUM(x,min,max) \
    if(x < min || max < x ) {\
        qInfo("%d not in[%d,%d]",x,min,max);\
        return false;\
    }

ApiHandler::ApiHandler()
{

}

bool ApiHandler::getVersion()
{
    QJsonObject jsonIn;
    QJsonObject jsonOut;
    int iRet = postJson("/api/v1/dev/getVersion",jsonIn,jsonOut);
    MemCache::instance()->setValue("app_version", jsonOut.value("app_version").toString());
    MemCache::instance()->setValue("mcu_version", jsonOut.value("mcu_version").toString());
    MemCache::instance()->setValue("nvs_version", jsonOut.value("nvs_version").toString());
    MemCache::instance()->setValue("version", jsonOut.value("version").toString());
    return iRet == 0? true:false;
}

bool ApiHandler::setFps(int fps)
{
    QJsonObject jsonIn;
    jsonIn.insert("value", fps);
    QJsonObject jsonOut;
    int iRet = postJson("/api/v1/dev/setFps",jsonIn,jsonOut);
    return iRet == 0? true:false;
}

bool ApiHandler::setNvsOn(bool bOn)
{
    QJsonObject jsonIn;
    jsonIn.insert("value", bOn ? 1 : 0);
    QJsonObject jsonOut;
    int iRet = postJson("/api/v1/dev/nvsOn",jsonIn,jsonOut);
    return iRet == 0 ? true:false;
}

bool ApiHandler::setScale(int scale)
{
    QJsonObject jsonIn;
    jsonIn.insert("value", scale);
    QJsonObject jsonOut;
    int iRet = postJson("/api/v1/dev/setScale",jsonIn,jsonOut);
    return iRet == 0 ? true:false;
}

bool ApiHandler::setTime()
{
    QJsonObject jsonIn;
    jsonIn.insert("value", QDateTime::currentDateTime().toString("yyyy_MM_dd_hh_mm_ss"));

    QJsonObject jsonOut;
    int iRet = postJson("/api/v1/dev/setTime",jsonIn,jsonOut);
    return iRet == 0 ? true:false;
}

bool ApiHandler::setFormat(int format,QString ip)
{
    QJsonObject jsonIn;
    jsonIn.insert("value", format);

    QJsonObject jsonOut;
    int iRet = postJson("/api/v1/sensor/setFormat",jsonIn,jsonOut,ip);
    return iRet == 0 ? true:false;
}

bool ApiHandler::setColor(int colorIdx,QString ip)
{
    QJsonObject jsonIn;
    CHECKNUM(colorIdx, 0, 26);

    jsonIn.insert("value", colorIdx);

    QJsonObject jsonOut;
    int iRet = postJson("/api/v1/sensor/colortable",jsonIn,jsonOut,ip);
    return iRet == 0 ? true:false;
}

bool ApiHandler::setRasel(int value)
{
    QJsonObject jsonIn;
    CHECKNUM(value, 0, 16);
    jsonIn.insert("value", value);
    QJsonObject jsonOut;
    int iRet = postJson("/api/v1/sensor/setRasel",jsonIn,jsonOut);
    return iRet == 0 ? true:false;
}

bool ApiHandler::setInt(int value)
{
    QJsonObject jsonIn;
    CHECKNUM(value, 0, 512);
    jsonIn.insert("value", value);
    QJsonObject jsonOut;
    int iRet = postJson("/api/v1/sensor/setInt",jsonIn,jsonOut);
    return iRet == 0 ? true:false;
}

bool ApiHandler::setGain(int value)
{
    QJsonObject jsonIn;
    CHECKNUM(value, 0, 15);
    jsonIn.insert("value", value);
    QJsonObject jsonOut;
    int iRet = postJson("/api/v1/sensor/setGain",jsonIn,jsonOut);
    return iRet == 0 ? true:false;
}

bool ApiHandler::setHssd(int value)
{
    QJsonObject jsonIn;
    CHECKNUM(value, 0, 255);
    jsonIn.insert("value", value);
    QJsonObject jsonOut;
    int iRet = postJson("/api/v1/sensor/setHssd",jsonIn,jsonOut);
    return iRet == 0 ? true:false;
}

bool ApiHandler::setNucUp(int value)
{
    QJsonObject jsonIn;
    CHECKNUM(value, 0, 16386);
    jsonIn.insert("value", value);
    QJsonObject jsonOut;
    int iRet = postJson("/api/v1/sensor/setNucUpper",jsonIn,jsonOut);
    return iRet == 0 ? true:false;
}

bool ApiHandler::setNucLow(int value)
{
    QJsonObject jsonIn;
    CHECKNUM(value, 0, 16386);
    jsonIn.insert("value", value);
    QJsonObject jsonOut;
    int iRet = postJson("/api/v1/sensor/setNucLower",jsonIn,jsonOut);
    return iRet == 0 ? true:false;
}

bool ApiHandler::setNucS(int value)
{
    QJsonObject jsonIn;
    CHECKNUM(value, 0, 255);
    jsonIn.insert("value", value);
    QJsonObject jsonOut;
    int iRet = postJson("/api/v1/sensor/setNucStart",jsonIn,jsonOut);
    return iRet == 0 ? true:false;
}

bool ApiHandler::setUpdateNucTemp(int value)
{
    QJsonObject jsonIn;
    CHECKNUM(value, 0, 255);
    jsonIn.insert("value", value);
    QJsonObject jsonOut;
    int iRet = postJson("/api/v1/sensor/updataNucTemp",jsonIn,jsonOut);
    return iRet == 0 ? true:false;
}

bool ApiHandler::setChangeNucTemp(int value)
{
    QJsonObject jsonIn;
    CHECKNUM(value, 0, 255);
    jsonIn.insert("value", value);
    QJsonObject jsonOut;
    int iRet = postJson("/api/v1/sensor/changeNucTemp",jsonIn,jsonOut);
    return iRet == 0 ? true:false;
}

bool ApiHandler::nucComp()
{
    QJsonObject jsonIn;
    QJsonObject jsonOut;
    int iRet = postJson("/api/v1/sensor/nucComp",jsonIn,jsonOut);
    return iRet == 0 ? true:false;
}

bool ApiHandler::shutterComp()
{
    QJsonObject jsonIn;
    QJsonObject jsonOut;
    int iRet = postJson("/api/v1/sensor/shutterComp",jsonIn,jsonOut);
    return iRet == 0 ? true:false;
}

bool ApiHandler::sceneComp()
{
    QJsonObject jsonIn;
    QJsonObject jsonOut;
    int iRet = postJson("/api/v1/sensor/sceneComp",jsonIn,jsonOut);
    return iRet == 0 ? true:false;
}

bool ApiHandler::setAutoCompTime(int value)
{
    QJsonObject jsonIn;
    CHECKNUM(value, 0, 100);
    jsonIn.insert("value", value);
    QJsonObject jsonOut;
    int iRet = postJson("/api/v1/sensor/autoComptimeTime",jsonIn,jsonOut);
    return iRet == 0 ? true:false;
}

bool ApiHandler::reset()
{
    QJsonObject jsonIn;
    QJsonObject jsonOut;
    int iRet = postJson("/api/v1/sensor/reset",jsonIn,jsonOut);
    return iRet == 0 ? true:false;
}

bool ApiHandler::saveConfig()
{
    QJsonObject jsonIn;
    QJsonObject jsonOut;
    int iRet = postJson("/api/v1/sensor/saveConfig",jsonIn,jsonOut);
    return iRet == 0 ? true:false;
}

bool ApiHandler::getSensorCfg(QJsonObject &json)
{
    QJsonObject jsonIn;
    int iRet = postJson("/api/v1/sensor/getSensorCfg",jsonIn,json);
    return iRet == 0 ? true:false;
}

bool ApiHandler::setSensorCfg(QJsonObject jsonIn)
{
    QJsonObject jsonOut;
    int iRet = postJson("/api/v1/sensor/setSensorCfg",jsonIn,jsonOut);
    return iRet == 0 ? true:false;
}

bool ApiHandler::uploadFile(QString sIp, uint8_t * pData, int uSize, QString &token)
{
    int sPort = HTTPPORT;

    HttpClient client(sIp.toStdString(), sPort);
    std::string sOut;

    int ret = client.put("/api/v1/dev/upload", pData, uSize, sOut);
    if(ret != RET_OK) {
        qInfo()<<"send error";
        return false;
    }
    qInfo("recv %s ", sOut.data());

    QJsonParseError jsonError;
    QJsonDocument document = QJsonDocument::fromJson(sOut.data(), &jsonError);
    if(document.isNull() || !document.isObject() || jsonError.error != QJsonParseError::NoError) {
        qInfo()<<"parse error";
        return false;
    }

    QJsonObject object = document.object();
    int iRet = -1;
    if(object.contains("code")) {
        if( 0 == object.value("code").toInt()) {
            iRet = 0;
            token = object.value("token").toString();
        }
    }
    return iRet == 0 ? true : false;
}

bool ApiHandler::upgrade(QString token, int type, int & iFailed, QString sIp)
{
    QJsonObject jsonIn;
    jsonIn.insert("type", type);
    jsonIn.insert("token", token);
    QJsonObject jsonOut;
    int iRet = postJson("/api/v1/dev/upgrade",jsonIn,jsonOut, sIp);
    if(iRet == HTTPERRORNO) {//收到回复，失败
        iFailed = 1;
    }
    return iRet == 0 ? true:false;
}

bool ApiHandler::isUpgraded(int &status, int &process, QString sIp)
{
    QJsonObject jsonIn;
    QJsonObject jsonOut;
    int iRet = postJson("/api/v1/dev/isupgraded",jsonIn,jsonOut, sIp);
    if(iRet == 0) {
        status = jsonOut.value("value").toInt();
        process = jsonOut.value("process").toInt();
    }
    return iRet == 0 ? true:false;
}

bool ApiHandler::saveBl(int count)
{
    QJsonObject jsonIn;
    jsonIn.insert("value", count);
    QJsonObject jsonOut;
    int iRet = postJson("/api/v1/sensor/saveBl",jsonIn,jsonOut);

    return iRet == 0 ? true:false;
}

bool ApiHandler::saveBh(int count)
{
    QJsonObject jsonIn;
    jsonIn.insert("value", count);
    QJsonObject jsonOut;
    int iRet = postJson("/api/v1/sensor/saveBh",jsonIn,jsonOut);

    return iRet == 0 ? true:false;
}

bool ApiHandler::calSaveK(QByteArray &kData)
{
    QJsonObject jsonIn;
    QJsonObject jsonOut;
    int iRet = postJson("/api/v1/sensor/calSaveK",jsonIn,jsonOut);
    if(!iRet && jsonOut.contains("value")) {
        kData = QByteArray::fromHex(jsonOut.value("value").toString().toUtf8());
    }
    return iRet == 0 ? true:false;
}

bool ApiHandler::setAutoGd(bool bOpen)
{
    QJsonObject jsonIn;
    jsonIn.insert("value", bOpen ? 1 : 0);
    QJsonObject jsonOut;
    int iRet = postJson("/api/v1/sensor/setAutoGd",jsonIn,jsonOut);
    return iRet == 0 ? true:false;
}

bool ApiHandler::saveX16(QByteArray &xData)
{
    QJsonObject jsonIn;
    QJsonObject jsonOut;
    int iRet = postJson("/api/v1/sensor/getImuX16",jsonIn,jsonOut);
    if(!iRet && jsonOut.contains("value")) {
        xData = QByteArray::fromHex(jsonOut.value("value").toString().toUtf8());
    }
    return iRet == 0 ? true:false;
}

bool ApiHandler::saveB(QByteArray &shutterValue, QByteArray &nvsValue)
{
    QJsonObject jsonIn;
    QJsonObject jsonOut;
    int iRet = postJson("/api/v1/sensor/saveB",jsonIn,jsonOut);
    if(!iRet && jsonOut.contains("shutter_value")) {
        shutterValue = QByteArray::fromHex(jsonOut.value("shutter_value").toString().toUtf8());
    }
    if(!iRet && jsonOut.contains("nvs_value")) {
        nvsValue = QByteArray::fromHex(jsonOut.value("nvs_value").toString().toUtf8());
    }
    return iRet == 0 ? true:false;
}

bool ApiHandler::loadKB(QByteArray &kData, QByteArray &bData)
{
    QJsonObject jsonIn;
    jsonIn.insert("kdata", QString(kData.toHex()));
    jsonIn.insert("bdata", QString(bData.toHex()));
    QJsonObject jsonOut;
    int iRet = postJson("/api/v1/sensor/loadKB",jsonIn,jsonOut);
    return iRet == 0 ? true:false;
}

bool ApiHandler::loadOcc(QByteArray &oData)
{
    QJsonObject jsonIn;
    jsonIn.insert("value", QString(oData.toHex()));
    QJsonObject jsonOut;
    int iRet = postJson("/api/v1/sensor/loadOcc",jsonIn,jsonOut);
    return iRet == 0 ? true:false;
}

bool ApiHandler::saveOcc(QByteArray &oData)
{
    QJsonObject jsonIn;
    QJsonObject jsonOut;
    int iRet = postJson("/api/v1/sensor/saveOcc",jsonIn,jsonOut);
    if(!iRet && jsonOut.contains("value")) {
        oData = QByteArray::fromHex(jsonOut.value("value").toString().toUtf8());
    }
    return iRet == 0 ? true:false;
}

bool ApiHandler::deltaOcc(QByteArray &oData)
{
    qDebug() << "deltaOcc size is " << oData.size();
    QJsonObject jsonIn;
    jsonIn.insert("value", QString(oData.toHex()));
    QJsonObject jsonOut;
    int iRet = postJson("/api/v1/dev/deltaOcc",jsonIn,jsonOut);
    return iRet == 0 ? true:false;
}

bool ApiHandler::getCenterAd(uint16_t &ad)
{
    QJsonObject jsonIn;
    QJsonObject jsonOut;
    int iRet = postJson("/api/v1/sensor/getCenterAd",jsonIn,jsonOut);
    if(!iRet && jsonOut.contains("value")) {
        ad = jsonOut.value("value").toInt();
    }
    return iRet == 0 ? true:false;
}

bool ApiHandler::getAd(uint16_t x, uint16_t y, uint16_t &ad)
{
    QJsonObject jsonIn;
    jsonIn.insert("x", x);
    jsonIn.insert("y", y);
    QJsonObject jsonOut;
    int iRet = postJson("/api/v1/sensor/getAd",jsonIn,jsonOut);
    if(!iRet && jsonOut.contains("value")) {
        ad = jsonOut.value("value").toInt();
    }
    return iRet == 0 ? true:false;
}

bool ApiHandler::getAvgAd(uint16_t &ad)
{
    QJsonObject jsonIn;
    QJsonObject jsonOut;
    int iRet = postJson("/api/v1/sensor/getAverageAd",jsonIn,jsonOut);
    if(!iRet && jsonOut.contains("value")) {
        ad = jsonOut.value("value").toInt();
    }
    return iRet == 0 ? true:false;
}

bool ApiHandler::setCalibInParam(QJsonObject json)
{
    QJsonObject jsonOut;
    int iRet = postJson("/api/v1/sensor/setCallibIn",json,jsonOut);
    return iRet == 0 ? true:false;
}

bool ApiHandler::getCalibInParam(QJsonObject &json)
{
    QJsonObject jsonIn;
    int iRet = postJson("/api/v1/sensor/getCallibIn",jsonIn,json);
    return iRet == 0 ? true:false;
}

bool ApiHandler::setCalibExParam(QJsonObject json)
{
    QJsonObject jsonOut;
    int iRet = postJson("/api/v1/sensor/setCallibEx",json,jsonOut);
    return iRet == 0 ? true:false;
}

bool ApiHandler::getCalibExParam(QJsonObject &json)
{
    QJsonObject jsonIn;
    int iRet = postJson("/api/v1/sensor/getCallibEx",jsonIn,json);
    return iRet == 0 ? true:false;
}

int ApiHandler::startCalibEx()
{
    QJsonObject jsonIn;
    QJsonObject jsonOut;
    int iRet = postJson("/api/v1/sensor/startCallibExt",jsonIn,jsonOut);
    int result = -1;
    if(jsonOut.contains("value")) {
        result = jsonOut.value("value").toInt();
    }
    return result;
}

bool ApiHandler::getAppWrite(QString &sValue)
{
    QJsonObject jsonIn;
    QJsonObject jsonOut;
    int iRet = postJson("/api/v1/sensor/appWrite",jsonIn,jsonOut);
    if(!iRet && jsonOut.contains("value")) {
        sValue = jsonOut.value("value").toString();
    }
    return iRet == 0 ? true:false;
}

bool ApiHandler::getAppLoad(QString &sValue)
{
    QJsonObject jsonIn;
    QJsonObject jsonOut;
    int iRet = postJson("/api/v1/sensor/appLoad",jsonIn,jsonOut);
    if(!iRet && jsonOut.contains("value")) {
        sValue = jsonOut.value("value").toString();
    }
    return iRet == 0 ? true:false;
}

bool ApiHandler::getAsicVersion(QString &sValue)
{
    QJsonObject jsonIn;
    QJsonObject jsonOut;
    int iRet = postJson("/api/v1/sensor/appVersion",jsonIn,jsonOut);
    if(!iRet && jsonOut.contains("value")) {
        sValue = jsonOut.value("value").toString();
    }
    return iRet == 0 ? true:false;
}

bool ApiHandler::getBootVersion(QString &sValue)
{
    QJsonObject jsonIn;
    QJsonObject jsonOut;
    int iRet = postJson("/api/v1/sensor/bootVersion",jsonIn,jsonOut);
    if(!iRet && jsonOut.contains("value")) {
        sValue = jsonOut.value("value").toString();
    }
    return iRet == 0 ? true:false;
}

bool ApiHandler::getDevType(QString &sValue)
{
    QJsonObject jsonIn;
    QJsonObject jsonOut;
    int iRet = postJson("/api/v1/sensor/devType",jsonIn,jsonOut);
    if(!iRet && jsonOut.contains("value")) {
        sValue = jsonOut.value("value").toString();
    }
    return iRet == 0 ? true:false;
}

bool ApiHandler::getHardVersion(QString &sValue)
{
    QJsonObject jsonIn;
    QJsonObject jsonOut;
    int iRet = postJson("/api/v1/sensor/hardVersion",jsonIn,jsonOut);
    if(!iRet && jsonOut.contains("value")) {
        sValue = jsonOut.value("value").toString();
    }
    return iRet == 0 ? true:false;
}

bool ApiHandler::getPattleVersion(QString &sValue)
{
    QJsonObject jsonIn;
    QJsonObject jsonOut;
    int iRet = postJson("/api/v1/sensor/colorizeVersion",jsonIn,jsonOut);
    if(!iRet && jsonOut.contains("value")) {
        sValue = jsonOut.value("value").toString();
    }
    return iRet == 0 ? true:false;
}

bool ApiHandler::getPowerOnImgVersion(QString &sValue)
{
    QJsonObject jsonIn;
    QJsonObject jsonOut;
    int iRet = postJson("/api/v1/sensor/poweronVersion",jsonIn,jsonOut);
    if(!iRet && jsonOut.contains("value")) {
        sValue = jsonOut.value("value").toString();
    }
    return iRet == 0 ? true:false;
}

bool ApiHandler::setDataType(int type)
{
    QJsonObject jsonIn;
    jsonIn.insert("value", type);
    QJsonObject jsonOut;
    int iRet = postJson("/api/v1/sensor/setDataType",jsonIn,jsonOut);
    return iRet == 0 ? true:false;
}

bool ApiHandler::calK()
{
    QJsonObject jsonIn;
    QJsonObject jsonOut;
    int iRet = postJson("/api/v1/sensor/calcK",jsonIn,jsonOut);
    return iRet == 0 ? true:false;
}

bool ApiHandler::saveK()
{
    QJsonObject jsonIn;
    QJsonObject jsonOut;
    int iRet = postJson("/api/v1/sensor/saveK",jsonIn,jsonOut);
    return iRet == 0 ? true:false;
}

bool ApiHandler::loadK()
{
    QJsonObject jsonIn;
    QJsonObject jsonOut;
    int iRet = postJson("/api/v1/sensor/loadK",jsonIn,jsonOut);
    return iRet == 0 ? true:false;
}

bool ApiHandler::loadDefK()
{
    QJsonObject jsonIn;
    QJsonObject jsonOut;
    int iRet = postJson("/api/v1/sensor/loadDefK",jsonIn,jsonOut);
    return iRet == 0 ? true:false;
}

bool ApiHandler::saveSffc()
{
    QJsonObject jsonIn;
    QJsonObject jsonOut;
    int iRet = postJson("/api/v1/sensor/saveSffc",jsonIn,jsonOut);
    return iRet == 0 ? true:false;
}

bool ApiHandler::collectSffc()
{
    QJsonObject jsonIn;
    QJsonObject jsonOut;
    int iRet = postJson("/api/v1/sensor/collectSffc",jsonIn,jsonOut);
    return iRet == 0 ? true:false;
}

bool ApiHandler::setSffc(bool bValue)
{
    QJsonObject jsonIn;
    jsonIn.insert("value", bValue ? 1:0);
    QJsonObject jsonOut;
    int iRet = postJson("/api/v1/sensor/setSffc",jsonIn,jsonOut);
    return iRet == 0 ? true:false;
}

bool ApiHandler::sceneNuc()
{
    QJsonObject jsonIn;
    QJsonObject jsonOut;
    int iRet = postJson("/api/v1/sensor/sceneNuc",jsonIn,jsonOut);
    return iRet == 0 ? true:false;
}

bool ApiHandler::setAutoCompStatus(bool bOpen)
{
    QJsonObject jsonIn;
    jsonIn.insert("value", bOpen ? 1:0);
    QJsonObject jsonOut;
    int iRet = postJson("/api/v1/sensor/autoComptimeStatus",jsonIn,jsonOut);
    return iRet == 0 ? true:false;
}

bool ApiHandler::rebootAsic()
{
    QJsonObject jsonIn;
    QJsonObject jsonOut;
    int iRet = postJson("/api/v1/sensor/hardReboot",jsonIn,jsonOut);
    return iRet == 0 ? true:false;
}

bool ApiHandler::debugUartSensor(QString sCmd, QByteArray &arr)
{
    QJsonObject jsonIn;
    jsonIn.insert("value",sCmd);
    QJsonObject jsonOut;
    int iRet = postJson("/api/v1/sensor/debug",jsonIn,jsonOut);
    if(!iRet && jsonOut.contains("value")) {
        arr = QByteArray::fromHex(jsonOut.value("value").toString().toUtf8());
    }
    return iRet == 0 ? true:false;
}

bool ApiHandler::setShutterOn(bool bValue)
{
    QJsonObject jsonIn;
    jsonIn.insert("value",bValue?1:0);
    QJsonObject jsonOut;
    int iRet = postJson("/api/v1/sensor/shutterControl",jsonIn,jsonOut);
    return iRet == 0 ? true:false;
}

bool ApiHandler::getAutoComtime(int &value)
{
    QJsonObject jsonIn;
    jsonIn.insert("value",value);
    QJsonObject jsonOut;
    int iRet = postJson("/api/v1/sensor/getAutoComTime",jsonIn,jsonOut);
    if(!iRet && jsonOut.contains("value")) {
        value = jsonOut.value("value").toInt();
    }
    return iRet == 0 ? true:false;
}

bool ApiHandler::sensorAdSingle(bool bValue)
{
    QJsonObject jsonIn;
    jsonIn.insert("value",bValue?1:0);
    QJsonObject jsonOut;
    int iRet = postJson("/api/v1/sensor/singleAd",jsonIn,jsonOut);
    return iRet == 0 ? true:false;
}

bool ApiHandler::hotRingStop()
{
    QJsonObject jsonIn;
    QJsonObject jsonOut;
    int iRet = postJson("/api/v1/sensor/tempUpStop",jsonIn,jsonOut);
    return iRet == 0 ? true:false;
}

bool ApiHandler::hotRingUp(float temp)
{
    QJsonObject jsonIn;
    jsonIn.insert("value",(int)(temp * 100));
    QJsonObject jsonOut;
    int iRet = postJson("/api/v1/sensor/tempUp",jsonIn,jsonOut);
    return iRet == 0 ? true:false;
}

bool ApiHandler::hotRingUP5s()
{
    QJsonObject jsonIn;
    QJsonObject jsonOut;
    int iRet = postJson("/api/v1/sensor/tempUp5s",jsonIn,jsonOut);
    return iRet == 0 ? true:false;
}

bool ApiHandler::downloadLog(QByteArray &logData)
{
    QJsonObject jsonIn;
    QJsonObject jsonOut;
    int iRet = postJson("/api/v1/dev/downloadLog",jsonIn,jsonOut);
    if(!iRet && jsonOut.contains("value")) {
        logData = QByteArray::fromHex(jsonOut.value("value").toString().toUtf8());
    }
    return iRet == 0 ? true:false;
}

bool ApiHandler::i2cDebugWrite(int devAddr, int addr, QByteArray sData)
{
    QJsonObject jsonIn;
    jsonIn.insert("devAddr", devAddr);
    jsonIn.insert("addr", addr);
    jsonIn.insert("value", QString(sData.toHex()).simplified());
    QJsonObject jsonOut;
    int iRet = postJson("/api/v1/i2c/debugWrite",jsonIn,jsonOut);
    if(!iRet ) {
    }
    return iRet == 0 ? true:false;
}

bool ApiHandler::i2cDebugRead(QByteArray &data, int devAddr, int addr, int rLen)
{
    QJsonObject jsonIn;
    jsonIn.insert("devAddr", devAddr);
    jsonIn.insert("addr", addr);
    jsonIn.insert("len", rLen);
    QJsonObject jsonOut;
    int iRet = postJson("/api/v1/i2c/debugRead",jsonIn,jsonOut);
    if(!iRet && jsonOut.contains("value")) {
        data = QByteArray::fromHex(jsonOut.value("value").toString().toUtf8());
    }
    return iRet == 0 ? true:false;
}

bool ApiHandler::setNoShutterAlg(bool bValue)
{
    QJsonObject jsonIn;
    jsonIn.insert("value", bValue ? 1 : 0);
    QJsonObject jsonOut;
    int iRet = postJson("/api/v1/dev/setShutterAlgStatus",jsonIn,jsonOut);
    return iRet == 0 ? true:false;
}

bool ApiHandler::getNoShutterAlg(bool &bValue)
{
    QJsonObject jsonIn;
    QJsonObject jsonOut;
    int iRet = postJson("/api/v1/dev/getShutterAlgStatus",jsonIn,jsonOut);
    if(!iRet && jsonOut.contains("value")) {
        bValue = jsonOut.value("value").toInt() ? true:false;
    }else {
        iRet = -1;
    }
    return iRet == 0 ? true:false;
}

bool ApiHandler::setOccBeforeValue(int value)
{
    QJsonObject jsonIn;
    jsonIn.insert("value", value);
    QJsonObject jsonOut;
    int iRet = postJson("/api/v1/dev/setOccBefore",jsonIn,jsonOut);
    return iRet == 0 ? true:false;
}

bool ApiHandler::getOccBeforeValue(int &value)
{
    QJsonObject jsonIn;
    QJsonObject jsonOut;
    int iRet = postJson("/api/v1/dev/getOccBefore",jsonIn,jsonOut);
    if(!iRet && jsonOut.contains("value")) {
        value = jsonOut.value("value").toInt();
    }else {
        iRet = -1;
    }
    return iRet == 0 ? true:false;
}

bool ApiHandler::setNoShutterBData(int temp, QByteArray arr)
{
    QJsonObject jsonIn;
    jsonIn.insert("temp", temp);
    jsonIn.insert("value", QString(arr.toHex()));
    QJsonObject jsonOut;
    int iRet = postJson("/api/v1/dev/setBData",jsonIn,jsonOut);

    return iRet == 0 ? true:false;
}

bool ApiHandler::getNoShutterBData(QVector<int> &vec)
{
    QJsonObject jsonIn;
    QJsonObject jsonOut;
    int iRet = postJson("/api/v1/dev/getCurBdata",jsonIn,jsonOut);
    if(!iRet && jsonOut.contains("list")) {
        QJsonArray jsonArr = jsonOut.value("list").toArray();
        for(int i = 0; i < jsonArr.size(); i ++) {
            int temp = jsonArr[i].toObject().value("temp").toInt();
            qInfo()<<"get temp "<<temp;
            vec.append(temp);
        }
    }else {
        iRet = -1;
    }
    return iRet == 0 ? true:false;
}

bool ApiHandler::setKData(QByteArray data)
{
    QJsonObject jsonIn;
    jsonIn.insert("value", QString(data.toHex()));
    QJsonObject jsonOut;
    int iRet = postJson("/api/v1/dev/setKData",jsonIn,jsonOut);

    return iRet == 0 ? true:false;
}

bool ApiHandler::clearNoShutterBData()
{
    QJsonObject jsonIn;
    QJsonObject jsonOut;
    int iRet = postJson("/api/v1/dev/clearBData",jsonIn,jsonOut);
    return iRet == 0 ? true:false;
}

bool ApiHandler::devReboot()
{
    QJsonObject jsonIn;
    QJsonObject jsonOut;
    int iRet = postJson("/api/v1/dev/reboot",jsonIn,jsonOut);
    return iRet == 0 ? true:false;
}

bool ApiHandler::startHeat(int value)
{
    QJsonObject jsonIn;
    jsonIn.insert("value",value);
    QJsonObject jsonOut;
    int iRet = postJson("/api/v1/dev/heatStart",jsonIn,jsonOut);
    return iRet == 0 ? true:false;
}

bool ApiHandler::stopHeat()
{
    QJsonObject jsonIn;
    QJsonObject jsonOut;
    int iRet = postJson("/api/v1/dev/heatStop",jsonIn,jsonOut);
    return iRet == 0 ? true:false;
}

bool ApiHandler::manuOcc()
{
    QJsonObject jsonIn;
    QJsonObject jsonOut;
    int iRet = postJson("/api/v1/dev/manualOcc",jsonIn,jsonOut);
    return iRet == 0 ? true:false;
}

bool ApiHandler::setAutoShutterCom(bool bValue)
{
    QJsonObject jsonIn;
    jsonIn.insert("value",bValue? 1:0);
    QJsonObject jsonOut;
    int iRet = postJson("/api/v1/dev/setShutterCom",jsonIn,jsonOut);
    return iRet == 0 ? true:false;
}

bool ApiHandler::loadDefNuc()
{
    QJsonObject jsonIn;
    QJsonObject jsonOut;
    int iRet = postJson("/api/v1/sensor/loadDefaultNuc",jsonIn,jsonOut);
    return iRet == 0 ? true:false;
}

bool ApiHandler::manuRasel()
{
    QJsonObject jsonIn;
    QJsonObject jsonOut;
    int iRet = postJson("/api/v1/sensor/manualRasel",jsonIn,jsonOut);
    return iRet == 0 ? true:false;
}

bool ApiHandler::manuHssd()
{
    QJsonObject jsonIn;
    QJsonObject jsonOut;
    int iRet = postJson("/api/v1/sensor/manualHssd",jsonIn,jsonOut);
    return iRet == 0 ? true:false;
}

bool ApiHandler::getOccInfo(OccInfo &info)
{
    QJsonObject jsonIn;
    QJsonObject jsonOut;
    int iRet = postJson("/api/v1/sensor/getOccCfg",jsonIn,jsonOut);
    if(jsonOut.contains("frmInterval")) {
        info.u32FrmInterval =
                jsonOut.value("frmInterval").toInt();
    }
    if(jsonOut.contains("calibFrmCnt")) {
        info.u16CalibFrmCnt =
                jsonOut.value("calibFrmCnt").toInt();
    }
    if(jsonOut.contains("calibTrend")) {
        info.u16CalibTrend =
                jsonOut.value("calibTrend").toInt();
    }
    if(jsonOut.contains("calibStepLen")) {
        info.u16CalibStepLen =
                jsonOut.value("calibStepLen").toInt();
    }
    if(jsonOut.contains("lowThrVal")) {
        info.u16LowThrVal =
                jsonOut.value("lowThrVal").toInt();
    }
    if(jsonOut.contains("highThrVal")) {
        info.u16HighThrVal =
                jsonOut.value("highThrVal").toInt();
    }
    if(jsonOut.contains("initVal")) {
        info.u16InitVal =
                jsonOut.value("initVal").toInt();
    }
    return iRet == 0 ? true:false;

}

bool ApiHandler::setOccInfo(OccInfo info)
{
    QJsonObject jsonIn;
    jsonIn.insert("frmInterval", (qint64)info.u32FrmInterval);
    jsonIn.insert("calibFrmCnt", info.u16CalibFrmCnt);
    jsonIn.insert("calibTrend", info.u16CalibTrend);
    jsonIn.insert("calibStepLen", info.u16CalibStepLen);
    jsonIn.insert("lowThrVal", info.u16LowThrVal);
    jsonIn.insert("highThrVal", info.u16HighThrVal);
    jsonIn.insert("initVal", info.u16InitVal);

    QJsonObject jsonOut;
    int iRet = postJson("/api/v1/sensor/setOccCfg",jsonIn,jsonOut);
    return iRet == 0 ? true:false;

}

bool ApiHandler::addBadPoint(const int x, const int y)
{
    QJsonObject jsonOut;
    QJsonObject jsonX;
    jsonX.insert("value", x);
    int iRet = postJson("/api/v1/sensor/badpoint/x",jsonX,jsonOut);
    QJsonObject jsonY;
    jsonY.insert("value", y);
    iRet = postJson("/api/v1/sensor/badpoint/y",jsonY,jsonOut);
    QJsonObject jsonIn;
    iRet = postJson("/api/v1/sensor/badpoint/add",jsonIn,jsonOut);
    return iRet == 0 ? true:false;
}

bool ApiHandler::saveBadPoint()
{
    QJsonObject jsonIn;
    QJsonObject jsonOut;
    int iRet = postJson("/api/v1/sensor/badpoint/save",jsonIn,jsonOut);
    return iRet == 0 ? true:false;
}

bool ApiHandler::deleteBadPoint()
{
    QJsonObject jsonIn;
    QJsonObject jsonOut;
    int iRet = postJson("/api/v1/sensor/badpoint/deleteAll",jsonIn,jsonOut);
    return iRet == 0 ? true:false;
}

bool ApiHandler::deleteCurrentBadPoint()
{
    QJsonObject jsonIn;
    QJsonObject jsonOut;
    int iRet = postJson("/api/v1/sensor/badpoint/deleteCurrent",jsonIn,jsonOut);
    return iRet == 0 ? true:false;
}

bool ApiHandler::badPointK(const int value)
{
    QJsonObject jsonIn;
    QJsonObject jsonOut;
    jsonIn.insert("value", value);
    int iRet = postJson("/api/v1/sensor/badpoint/k",jsonIn,jsonOut);
    return iRet == 0 ? true:false;
}

bool ApiHandler::badPointY16(const int value)
{
    QJsonObject jsonIn;
    QJsonObject jsonOut;
    jsonIn.insert("value", value);
    int iRet = postJson("/api/v1/sensor/badpoint/y16",jsonIn,jsonOut);
    return iRet == 0 ? true:false;
}

bool ApiHandler::getCurBadPointCount(int &count)
{
    QJsonObject jsonIn;
    QJsonObject jsonOut;
    int iRet = postJson("/api/v1/sensor/getCurBadCount",jsonIn,jsonOut);
    if(!iRet && jsonOut.contains("value")) {
        count = jsonOut.value("value").toInt();
    }
    return iRet == 0 ? true:false;
}

bool ApiHandler::getAllBadPointCount(int &count)
{
    QJsonObject jsonIn;
    QJsonObject jsonOut;
    int iRet = postJson("/api/v1/sensor/getBadTotalCount",jsonIn,jsonOut);
    if(!iRet && jsonOut.contains("value")) {
        count = jsonOut.value("value").toInt();
    }
    return iRet == 0 ? true:false;
}

bool ApiHandler::getBadpointList(QVector<QPoint>& vec)
{
    QJsonObject jsonIn;
    QJsonObject jsonOut;
    int iRet = postJson("/api/v1/sensor/getBadList",jsonIn,jsonOut);
    if(!iRet && jsonOut.contains("list")) {
        QJsonArray array = jsonOut.value("list").toArray();
        for(int i=0; i<array.size(); i++) {
            QJsonObject obj = array[i].toObject();
            QPoint point;
            point.setX(obj.value("x").toInt());
            point.setY(obj.value("y").toInt());
            vec.append(point);
        }
    }
    return iRet == 0 ? true:false;
}

bool ApiHandler::setBadpointList(QVector<QPoint> vec, const int type)
{
    QJsonObject jsonIn;
    QJsonObject jsonOut;

    QJsonObject list;
    QJsonArray array;
    for(int i=0; i<vec.size(); i++) {
        list.insert("x", vec[i].x());
        list.insert("y", vec[i].y());
        array.append(list);
    }
    jsonIn.insert("type", type);
    jsonIn.insert("list", array);
    int iRet = postJson("/api/v1/sensor/setBadList",jsonIn,jsonOut);
    return iRet == 0 ? true:false;
}

bool ApiHandler::setIpsParam(int id, QString data)
{
    QJsonObject jsonIn;
    QJsonObject jsonOut;
    jsonIn.insert("isp_id", id);
    jsonIn.insert("data", data);
    int iRet = postJson("/api/v1/sensor/setIsp",jsonIn,jsonOut);
    return iRet == 0 ? true:false;
}

bool ApiHandler::getIpsParam(int id, int len, QString& data)
{
    QJsonObject jsonIn;
    QJsonObject jsonOut;
    jsonIn.insert("isp_id", id);
    jsonIn.insert("len", len);
    int iRet = postJson("/api/v1/sensor/getIsp",jsonIn,jsonOut);
    if(!iRet && jsonOut.contains("data")) {
        data = jsonOut.value("data").toString();
    }
    return iRet == 0 ? true:false;
}

bool ApiHandler::getFocusTemp(float &temp)
{
    QJsonObject jsonIn;
    QJsonObject jsonOut;
    int iRet = postJson("/api/v1/sensor/getFocusTemp",jsonIn,jsonOut);
    if(!iRet && jsonOut.contains("value")) {
        temp = jsonOut.value("value").toDouble();
    }
    return iRet == 0 ? true:false;
}

bool ApiHandler::setFusionSwitch(const bool flag,QString ip)
{
    QJsonObject jsonIn;
    QJsonObject jsonOut;
    jsonIn.insert("value", flag ? 1 : 0);
    int iRet = postJson("/api/v1/sensor/setFusion",jsonIn,jsonOut,ip);
    return iRet == 0 ? true:false;
}

bool ApiHandler::startImuCalib()
{
    QJsonObject jsonIn;
    QJsonObject jsonOut;
    int iRet = postJson("/api/v1/sensor/startImuCalib",jsonIn,jsonOut);

    int status;
    iRet = getImuCalibStatus(status);
    qDebug() << status;
    return status == 1;
}

bool ApiHandler::getImuCailbInf(QString& txt)
{
    QJsonObject jsonIn;
    QJsonObject jsonOut;
    int iRet = postJson("/api/v1/sensor/getImuCalibInfo",jsonIn,jsonOut);
    txt = QString(QJsonDocument(jsonOut).toJson(QJsonDocument::Indented));
    return iRet;
}

bool ApiHandler::getImuCalibStatus(int& flag)
{
    QJsonObject jsonIn;
    QJsonObject jsonOut;
    int iRet = postJson("/api/v1/sensor/isImuCalib",jsonIn,jsonOut);
    if(!iRet && jsonOut.contains("value")) {
        flag = jsonOut.value("value").toInt();
    }
    return iRet;
}

bool ApiHandler::setAlgSwitch(const int index, int value)
{
    QJsonObject jsonIn;
    jsonIn.insert("addr", index+1);
    jsonIn.insert("value", value);
    QJsonObject jsonOut;
    int iRet = postJson("/api/v1/sensor/algorithm",jsonIn,jsonOut);
    if(!iRet ) {
    }
    return iRet == 0 ? true:false;
}

bool ApiHandler::setFusionPattle(const int index)
{
    QJsonObject jsonIn;
    QJsonObject jsonOut;
    jsonIn.insert("value", index);
    int iRet = postJson("/api/v1/dev/setFusionColor",jsonIn,jsonOut);
    return iRet == 0 ? true:false;
}

bool ApiHandler::setAlgCustomData(QString txt)
{
    QJsonObject jsonIn;
    QJsonObject jsonOut;
    jsonIn.insert("value", txt);
    int iRet = postJson("/api/v1/dev/setAlgStore",jsonIn,jsonOut);
    return iRet == 0 ? true:false;
}

bool ApiHandler::getAlgCustomData(QString &txt)
{
    QJsonObject jsonIn;
    QJsonObject jsonOut;
    int iRet = postJson("/api/v1/dev/getAlgStore",jsonIn,jsonOut);
    if(!iRet && jsonOut.contains("value")) {
        txt = jsonOut.value("value").toString();
    }
    return iRet == 0 ? true:false;
}

bool ApiHandler::setSn(const QString sn)
{
    QJsonObject jsonIn;
    QJsonObject jsonOut;
    jsonIn.insert("value", sn);
    int iRet = postJson("/api/v1/sensor/setSn",jsonIn,jsonOut);
    return iRet == 0 ? true:false;
}

bool ApiHandler::getSn(QString &sn)
{
    QJsonObject jsonIn;
    QJsonObject jsonOut;
    int iRet = postJson("/api/v1/sensor/getSn",jsonIn,jsonOut);
    if(!iRet && jsonOut.contains("value")) {
        sn = jsonOut.value("value").toString();
    }
    return iRet == 0 ? true:false;
}

bool ApiHandler::deltbOpen()
{
    QJsonObject jsonIn;
    QJsonObject jsonOut;
    int iRet = postJson("/api/v1/sensor/opendeltb",jsonIn,jsonOut);
    return iRet == 0 ? true:false;
}

bool ApiHandler::deltbClose()
{
    QJsonObject jsonIn;
    QJsonObject jsonOut;
    int iRet = postJson("/api/v1/sensor/closedeltb",jsonIn,jsonOut);
    return iRet == 0 ? true:false;
}

bool ApiHandler::deltbSave()
{
    QJsonObject jsonIn;
    QJsonObject jsonOut;
    int iRet = postJson("/api/v1/sensor/savedeltb",jsonIn,jsonOut);
    return iRet == 0 ? true:false;
}

bool ApiHandler::deltbCollect()
{
    QJsonObject jsonIn;
    QJsonObject jsonOut;
    int iRet = postJson("/api/v1/sensor/collectdeltb",jsonIn,jsonOut);
    return iRet == 0 ? true:false;
}

bool ApiHandler::getAddrTest(QJsonObject &json)
{
    QJsonObject jsonIn;
    int iRet = postJson("/api/v1/sensor/communicationtest",jsonIn,json);
    return iRet == 0 ? true:false;
}

bool ApiHandler::setNucDefaultValue(const int value)
{
    QJsonObject jsonIn;
    QJsonObject jsonOut;
    jsonIn.insert("value", value);
    int iRet = postJson("/api/v1/sensor/nucset",jsonIn,jsonOut);
    return iRet == 0 ? true:false;
}

bool ApiHandler::setAlgSceneCom(const int flag)
{
    QJsonObject jsonIn;
    QJsonObject jsonOut;
    jsonIn.insert("value", flag);
    int iRet = postJson("/api/v1/sensor/setAlgSceneCom",jsonIn,jsonOut);
    return iRet == 0 ? true:false;
}

bool ApiHandler::setSucEnable(const int flag)
{
    QJsonObject jsonIn;
    QJsonObject jsonOut;
    jsonIn.insert("value", flag);
    int iRet = postJson("/api/v1/sensor/algorithm/sucSwitch",jsonIn,jsonOut);
    return iRet == 0 ? true:false;
}

bool ApiHandler::setSucCollect(QByteArray data)
{
    QJsonObject jsonIn;
    QJsonObject jsonOut;
    jsonIn.insert("addr", 0x34);
    jsonIn.insert("value", QString::fromStdString(data.toStdString()));
    int iRet = postJson("/api/v1/sensor/sucset",jsonIn,jsonOut);
    return iRet == 0 ? true:false;
}

int ApiHandler::postJson(QString uri, QJsonObject &jsonin, QJsonObject &jsonOut, QString sIp)
{
    if(sIp.length() <= 0) {
        // SystemInfo sinfo;
        // ConfigInfo::instance()->GetSystemInfo(sinfo);
        // sIp = sinfo.ip;
    }
    int sPort = HTTPPORT;

    HttpClient client(sIp.toStdString(), sPort);
    std::string sOut;

    int ret = client.post(uri.toStdString(), JSONTOSTR(jsonin), sOut);
    if(ret != RET_OK) {
        return -2;
    }
    qInfo("recv %s ", sOut.substr(0, 1024).data());

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
        }else {
            iRet = HTTPERRORNO;
        }
    }
    jsonOut = object;
    return iRet;
}

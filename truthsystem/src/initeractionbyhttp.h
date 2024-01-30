#ifndef APIHANDLER_H
#define APIHANDLER_H
#include <configinfo.h>
#include <QJsonObject>
#include <singleton.h>
#include <QJsonValue>

struct OccInfo{
    uint32_t u32FrmInterval;
    uint16_t u16CalibFrmCnt;
    uint16_t u16CalibTrend;
    uint16_t u16CalibStepLen;
    uint16_t u16LowThrVal;
    uint16_t u16HighThrVal;
    uint16_t u16InitVal;
};

class ApiHandler:public Singleton<ApiHandler>
{
public:
    ApiHandler();

    bool getVersion();
    bool setFps(int fps);
    bool setNvsOn(bool bOn);
    bool setScale(int scale);
    bool setTime();
    bool setFormat(int format,QString ip);
    bool setColor(int colorIdx,QString ip);
    bool setRasel(int value);
    bool setInt(int value);
    bool setGain(int value);
    bool setHssd(int value);
    bool setNucUp(int value);
    bool setNucLow(int value);
    bool setNucS(int value);
    bool setUpdateNucTemp(int value);
    bool setChangeNucTemp(int value);

    bool nucComp();
    bool shutterComp();
    bool sceneComp();

    bool setAutoCompTime(int value);
    bool reset();
    bool saveConfig();
    bool getSensorCfg(QJsonObject &json);
    bool setSensorCfg(QJsonObject json);

    bool uploadFile(QString sIp, uint8_t *pData, int uSize, QString &token);
    bool upgrade(QString token, int type, int &iFailed, QString sIp = "");
    bool isUpgraded(int &status, int &process, QString sIp = "");

    bool saveBl(int count);
    bool saveBh(int count);
    bool calSaveK(QByteArray & kData);
    bool setAutoGd(bool bOpen);
    bool saveX16(QByteArray & xData);
    bool saveB(QByteArray & shutterValue, QByteArray &nvsValue);
    bool loadKB(QByteArray &kData, QByteArray &bData);
    bool loadOcc(QByteArray &oData);
    bool saveOcc(QByteArray &oData);
    bool deltaOcc(QByteArray &oData);
    bool getCenterAd(uint16_t & ad);
    bool getAd(uint16_t x, uint16_t y, uint16_t & ad);
    bool getAvgAd(uint16_t & ad);


    bool setCalibInParam(QJsonObject json);
    bool getCalibInParam(QJsonObject& json);
    bool setCalibExParam(QJsonObject json);
    bool getCalibExParam(QJsonObject& json);
    int startCalibEx();

	bool getAppWrite(QString &sValue);
    bool getAppLoad(QString &sValue);
    bool getAsicVersion(QString &sValue);
    bool getBootVersion(QString &sValue);
    bool getDevType(QString &sValue);
    bool getHardVersion(QString &sValue);
    bool getPattleVersion(QString &sValue);
    bool getPowerOnImgVersion(QString &sValue);

    bool setDataType(int type);
    bool calK();
    bool saveK();
    bool loadK();
    bool loadDefK();
    bool saveSffc();
    bool collectSffc();
    bool setSffc(bool bValue);
    bool sceneNuc();
    bool setAutoCompStatus(bool bOpen);
    bool rebootAsic();
    bool debugUartSensor(QString sCmd, QByteArray &arr);

    bool addBadPoint(const int x, const int y);
    bool saveBadPoint();
    bool deleteBadPoint();
    bool deleteCurrentBadPoint();
    bool badPointK(const int value);
    bool badPointY16(const int value);
    bool getCurBadPointCount(int& count);
    bool getAllBadPointCount(int& count);
    bool getBadpointList(QVector<QPoint>& vec);
    bool setBadpointList(QVector<QPoint> vec, const int type);
    bool setShutterOn(bool bValue);
    bool getAutoComtime(int &value);
    bool sensorAdSingle(bool bValue);
    bool hotRingStop();
    bool hotRingUp(float temp);
    bool hotRingUP5s();
    bool downloadLog(QByteArray & logData);
    bool i2cDebugWrite(int devAddr, int addr, QByteArray sData);
    bool i2cDebugRead(QByteArray &data, int devAddr, int addr, int rLen );
    bool setNoShutterAlg(bool bValue);
    bool getNoShutterAlg(bool & bValue);
    bool setOccBeforeValue(int value);
    bool getOccBeforeValue(int &value);
    bool setNoShutterBData(int temp, QByteArray  arr);
    bool getNoShutterBData(QVector<int> & vec);
    bool setKData(QByteArray data);
    bool clearNoShutterBData();
    bool devReboot();
    bool startHeat(int value);
    bool stopHeat();
    bool manuOcc();
    bool setAutoShutterCom(bool bValue);
    bool loadDefNuc();
    bool manuRasel();
    bool manuHssd();
    bool getOccInfo(OccInfo & info);
    bool setOccInfo(OccInfo info);

    bool setIpsParam(int id, QString data);
    bool getIpsParam(int id, int len, QString& data);
    bool getFocusTemp(float& temp);
    bool setFusionSwitch(const bool flag,QString ip);

    bool startImuCalib();
    bool getImuCailbInf(QString& txt);
    bool getImuCalibStatus(int& flag);

    bool setAlgSwitch(const int index, int value);

    bool setFusionPattle(const int index);
    bool setAlgCustomData(QString txt);
    bool getAlgCustomData(QString& txt);
    bool setSn(const QString sn);
    bool getSn(QString& sn);

    bool deltbOpen();
    bool deltbClose();
    bool deltbSave();
    bool deltbCollect();
    bool getAddrTest(QJsonObject& json);
    bool setNucDefaultValue(const int value);
    bool setAlgSceneCom(const int flag);
    bool setSucEnable(const int flag);
    bool setSucCollect(QByteArray data);

private:
    int postJson(QString uri, QJsonObject &jsonin, QJsonObject & jsonOut, QString sIp = "");
};

#endif // APIHANDLER_H

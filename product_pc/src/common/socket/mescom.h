#ifndef MESCOM_H
#define MESCOM_H
#include <mesclient.h>
#include <configinfo.h>
#include <comdll.h>

#define  MESPASS "PASS"
#define  MESFAILED "NG"

enum SnType{
    // 整机SN
    SnType_Mobile = 0,
    // 包装SN
    SnType_Package = 1
};

class Q_API_FUNCTION MesCom:public Singleton<MesCom>
{
public:
    MesCom();
    //设置用户信息
    void setUser(QString sUser);
    int login(QString user, QString passwd);
    //获取sn
    //sOrderId 工单号
    //result sn
    //return 0 成功 其他失败
    int getOrderSns(QString sOrderId, QString &result, bool is_package = false, bool isGetNew = false);
    //进站操作
    //sn 进站编码
    //sOutMsg 进站失败信息
    //return
    bool enterProduce(QString sn, QString &sOutMsg, QString processCode = "");

    // 获取未完成数量和完成数量
    int getFinishedQuantity();
    int getUnFinishedQuantity();

    QString getUser();
    QString getPasswd();


    //出站操作
    //sOutMsg 出站失败信息
    //return
    bool outProduce(int result , QString &sOutMsg, QString sn = "");

    //设置检测结果信息,需要在进站后 出站前调用，工序里面设置了检测项的必须调用，并设置，否则出站失败
    void setItemResult(MesCheckItem * pItem);
    void addItemResult(QString key, QString value, bool rst, QString err = "");
    void addItemResultEx(MesCheckItem pItem);
    void clearItem();
    //获取坏点信息,此接口必须在进站后调用，否则失败
    //sId sn,或者探测器id
    //outData 159,42 584,146
    bool getBadPointInfo(QString sId, QStringList &outData, QString &sSensorSn);

    // 获取版本文件
    // asic/arm/mcu
    bool getVersionFile(QString savePath, QString type = "asic");
    bool getUpgradeFileVersion(QString &version, QString type = "asic");
    QMap<QString, MesVersionFileInfo> fileInfo() const;
    //获取整机模版,指定路径
    bool getPrintFile(QString &savePath);
    //获取外箱模版,指定路径
    bool getPackagePrintFile(QString &savePath);
    //获取检测项数值
    bool getCheckInfo(QString itemName, MesCheckItem &item);
    //获取绑定物料信息
    bool getMetrialList(QString itemId, QList<MaterialInfo> &list);

    bool getSensorSN(QString mobileSn, QString& sensorSn, QString keyWord = "sensor_sn");

    bool getSnByMaterialSn(QString materialSn, QString& sn);

    bool packageVerifySn(QString packageSn, QString sn);
    bool packageBindSns(QString packageSn, QStringList sns);
    QString getMaterialAACode(QString order);
    QString getOrderId();
    QString errString();
protected:
    QMap<QString, MesCheckItem> m_mapData;
    QMap<QString, MesMaterItem> m_materData;
    int m_enterSuc = 0;
private:
    MesClient m_client;
    QString m_user;
    QMap<QString, MesVersionFileInfo> m_fileInfo;

private:
    void printMapData();
    void printMasterData();
};

#endif // MESCOM_H

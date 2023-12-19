#ifndef MESCLIENT_H
#define MESCLIENT_H
#include <qjsonobject.h>
#include <qlist.h>
#include <qmap.h>
#include <comdll.h>

#include <singleton.h>
#include <interfaceconstdef.h>
#include <QString>
#include <functional>

#define MESMAXCHECKNUM 100

struct MesCheckItem{
    QString sItem;
    QString sValue;
    QString sMin;
    QString sMax;
    QString sExp;
    QString sResult;//"PASS" "NG"
    QString tCheckTm;
    QString sNote;
};

struct MesMaterItem {
    QString sId;
    QString sName;
    QString sValue;
    QString sBarCode;
    QString sFileUrl;
};

// download file infor
struct MesVersionFileInfo {
    QString sName;
    QString sVersion;
    QString sToken;
    void clear() {
        sName = "";
        sVersion = "";
        sToken = "";
    }
};

// 获取工单信息
typedef struct {
    int planQuality;
    QString orderId;
    QString productCode;
    QString productName;
    void clear() {
        planQuality = 0;
        orderId.clear();
        productCode.clear();
        productName.clear();
    }
} OrderInformation;

struct MaterialInfo{//物料信息
    QString mesBarCode;
    QString basMaterialName;
    QString basMaterialCode;
    QString basMaterialSprc;
    QString sn;
};

typedef std::function<int(MesCheckItem)> MesFuncCheckItem;

struct MesCheckPackage{
    QString sItem;
};

class Q_API_FUNCTION MesClient:public Singleton<MesClient>
{
public:
    MesClient();
    int login(QString &sUserName, QString &sPass);

    int getStardData(QList<ResultInfo> &listData);

    int enterProduce(QString sn,
                     QMap<QString, MesCheckItem> &mapData,
                     QMap<QString, MesMaterItem> &materItem,
                     QMap<QString, MesVersionFileInfo> & fileInfo,
                     QString processCode = "");

    int outProduce(QMap<QString, MesCheckItem> &mapData,
                   QMap<QString, MesMaterItem> & materItem,
                   QString sResult, QString &sMsgOut, QString sn = "");

    int uploadData(QString &sOutToken, QByteArray &array);

    int downloadData(QString &sToken, QByteArray &array, QString sKeyName = "xuanyuan");
    QString sUser() const;
    void setSUser(const QString &newSUser);
    QString sPass() const;
    void setSPass(const QString &newSPass);

    int getOrderIds( QList<ResultInfo> &result);

    int getOrderSns(QString sOrderId,  QString &result, bool isPackage = false, bool isCreateNewEveryTime = false);

    int getSensorData(QString sn, QStringList &result, QString &sSenserSn);
    QString productPrintTemplateUrl() const;
    QString packagePrintTemplateUrl() const;
    QString orderCode() const;
    QMap<QString, MesVersionFileInfo> fileInfo() const;

    int getSnList(QString sItem, QList<MaterialInfo> &list);
    int getSnByMaterialSn(QString materialSn, QString& sn);

    int getOrderInfor(QString code);

    int packageCheckSn(QString packageSn, QString productSn);
    int packageBindSn(QString packageSn, QStringList productSns);

    QString errString();
    QString materialAACode(QString order);
    QString orderId() const;

private:
    QString m_sUser;
    QString m_sPass;
    QString m_workOrderId;
    QString m_productPrintTemplateUrl;
    QString m_packagePrintTemplateUrl;
    QString m_orderCode;
    QString m_materialAACode;
    QMap<QString, MesVersionFileInfo> m_fileInfo;
    QMap<QString, QString> m_mapSnMesOrderId;

    QString m_errString;
    QString m_deviceCode;

    OrderInformation m_orderInfor;
};

#endif // MESCLIENT_H

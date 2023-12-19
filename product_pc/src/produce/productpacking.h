#ifndef PRODUCTPACKING_H
#define PRODUCTPACKING_H



#include <QObject>
#include <mescom.h>
#include <QString>
#include <baseproduce.h>
#include "mcuserialmanager.h"
#include "ASicSerialManager.h"
#include <csvlogger.h>
#include "CMyPrinter.h"
#include <QTime>
#include <QFileInfo>
#include "electronicscale.h"

#include <vector>
using namespace std;


typedef struct {
    QString mobileSN;
    float heavy;
} PackageInfor, *PackageInforPtr;

typedef enum {
    WTypeSingleWeight = 0,
    WTypeLineFoam = 1, // 内衬泡棉
    WTypeCoverFoam = 2, // 上盖泡棉
    WTypeNakedWeight = 3,
    WTypeFullWeight = 4,
} WeightingType;

class productpacking: public BaseProduce
{
    Q_OBJECT
public:
    productpacking();

public slots:
    void slotsStoped();
    void slotNotFullPrint();
    void slotStartWorkBtnClicked();

    void slotConnectBtnClicked();

private:
    int m_totalCount = 40;
    int m_layoutCount = 2;
    int m_singleCount =20;

    QString m_projectName;

    QString m_packBoxSN;

    QString m_printTemplatePath;
    bool m_onlineTemplate = false;

    QString m_workOrder;
    QString m_workAACode;

    MesCom m_mesObj;

    QString mLogPath;


    int mPackageSeq = 0;

    ElectronicScale m_elecScale;
    QString m_elecSerial;

    QStringList m_snList;
    vector<PackageInfor> m_pkgInfors;
    float m_heavyNakeTotal = .0f;
    float m_heavyTotal = .0f;

    bool m_printAnyway = false;
private:
    bool getParam();

    void initParam();

    void beforePacking();
    void packing();
    void afterPacking();

    void deInit();

    void combineHeavy(QString name, float heavy, bool isProcuct = true);
    bool snValidCheck(QString sn);
    bool heavyValidCheck(WeightingType wt, float weight);

    bool print();

    bool scanCompare();

    bool bindPkgSnWithPSNs();

    void showProcessLog(QString str, int flag = 0);

    // About weighting...
    bool getSingleWeight(float& weight);
    bool getTotalWeight(float& weight);


public slots:
    void slotStopWorkBtnClicked();

public slots:

      void slotprintSN();
};





#endif // PRODUCTPACKING_H

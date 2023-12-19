#ifndef PRODUCEMANAGER_H
#define PRODUCEMANAGER_H
#include <singleton.h>
#include <QVector>
#include <baseproduce.h>

struct ProduceInfo {
    QString sName;
    QString sCode;
    int uiComponent;
};


class ProduceManager:public Singleton<ProduceManager>
{
public:
    ProduceManager();
    QVector<ProduceInfo> produces() const;

    int getUiTemplate(QString sCode);

    BaseProduce * getCurProduce();
    QString getTitle(QString sCode);

private:
    QVector<ProduceInfo> m_produces;
};

#endif // PRODUCEMANAGER_H

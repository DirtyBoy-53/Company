#include "RealTimeInfoWidget.h"
#include <QMap>
#include <QVariant>
#include <QDebug>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QTextEdit>

#include "ParamLineHandler.h"
#include "YTableWidget.h"
#include "YFunction.h"

RealTimeInfoWidget::RealTimeInfoWidget(QWidget *parent)
    : QWidget(parent)
    , tableWidget(new YTableWidget)
    , textEdit(new QTextEdit)
{
    initUI();
    initConnect();
}

RealTimeInfoWidget::~RealTimeInfoWidget()
{
}

void RealTimeInfoWidget::initUI()
{
    QMap<QString, QVariant> map;
    ParamLineHandler paramHandler;
    QStringList strList = paramHandler.getParamStr();
    if(strList.size() <= 0 ) return;
    for(auto it : strList){
        map[it] = QVariant(QString(""));
    }
    tableWidget->setData(map);

    auto splitter = new QSplitter(Qt::Horizontal);
    splitter->addWidget(tableWidget);
    splitter->addWidget(textEdit);
    auto hBox  = new QHBoxLayout(this);
    hBox->addWidget(splitter);
    // hBox->addWidget(tableWidget);
    // hBox->addSpacing(5);
    // hBox->addWidget(textEdit);

}

void RealTimeInfoWidget::initConnect()
{

}

void RealTimeInfoWidget::setMedia(Media &media)
{
    mMedia = media;
}

void RealTimeInfoWidget::setParamLine(QByteArray param)
{
    int index{0};
    QMap<QString, QVariant> map;
    ParamLineHandler paramHandler;
    QStringList strList = paramHandler.getParamStr();

    textEdit->setText(byteArrayToHexStr(param.mid(0,120)));

    if(strList.size() <= 0 ) return;
    paramHandler.parseParamLine(param);
    ParamLineInfo paramInfo = paramHandler.getParamInfo();

    map[strList.at(index++)] = QVariant(QString::number(paramInfo.shutterFlag));
    map[strList.at(index++)] = QVariant(QString::number(paramInfo.shutterMode));
    map[strList.at(index++)] = QVariant(QString::number(paramInfo.closeFlag));
    map[strList.at(index++)] = QVariant(QString::number(paramInfo.nucFlag));
    map[strList.at(index++)] = QVariant(QString::number(paramInfo.adFlag));
    map[strList.at(index++)] = QVariant(QString::number(paramInfo.autoFlag));
    map[strList.at(index++)] = QVariant(QString::number(paramInfo.colorType));
    map[strList.at(index++)] = QVariant(QString::number(paramInfo.freezeStatus));
    map[strList.at(index++)] = QVariant(QString::number((float)paramInfo.jTemp/100.0));
    map[strList.at(index++)] = QVariant(QString::number(paramInfo.outputMode));
    map[strList.at(index++)] = QVariant(QString::number(paramInfo.sceneMode));
    map[strList.at(index++)] = QVariant(QString::number(paramInfo.comType));
    map[strList.at(index++)] = QVariant(QString::number(paramInfo.hist));
    map[strList.at(index++)] = QVariant(QString::number(paramInfo.avgAd));
    map[strList.at(index++)] = QVariant(QString::number(paramInfo.adc0));
    map[strList.at(index++)] = QVariant(QString::number(paramInfo.adc1));
    map[strList.at(index++)] = QVariant(QString::number((float)paramInfo.ringTemp/100.0));
    map[strList.at(index++)] = QVariant(QString::number((float)paramInfo.pcbTemp/100.0));
    map[strList.at(index++)] = QVariant(QString::number(paramInfo.ringState));
    map[strList.at(index++)] = QVariant(QString::number(paramInfo.ringSwitch));
    // map[strList.at(index++)] = QVariant(QString::number(paramInfo.light));
    // map[strList.at(index++)] = QVariant(QString::number(paramInfo.contrast));
    // map[strList.at(index++)] = QVariant(QString::number(paramInfo.enhance));
    // map[strList.at(index++)] = QVariant(QString::number(paramInfo.doNoise));
    // map[strList.at(index++)] = QVariant(QString::number(paramInfo.bigGear));
    // map[strList.at(index++)] = QVariant(QString::number(paramInfo.smallGearL));
    // map[strList.at(index++)] = QVariant(QString::number(paramInfo.smallGearH));
    QString errCode = QString::asprintf("%02d %02d %02d %02d %02d %02d %02d %02d",
                                        paramInfo.errCode[0],paramInfo.errCode[1],paramInfo.errCode[2],paramInfo.errCode[3],
                                        paramInfo.errCode[4],paramInfo.errCode[5],paramInfo.errCode[6],paramInfo.errCode[7]);
    map[strList.at(index++)] = QVariant(errCode);

    tableWidget->setData(map);

}

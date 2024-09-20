#include "ISPParamWidget.h"
#include "YTableWidget.h"

#include <QComboBox>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QPushButton>
#include <exception>
#include <QDebug>
#include <algorithm>

#include "AlgConst.h"
#include "I2CManager.h"
ISPParamWidget::ISPParamWidget(QWidget *parent)
    : QWidget(parent)
{
    initUI();
    initConnect();
}

void ISPParamWidget::setMedia(Media &media)
{
    mMedia = media;
}

void ISPParamWidget::initUI()
{
    tableWidget = new YTableWidget;
    auto asicBox = new QComboBox();
    auto paramBox = new QComboBox();

    asicBox->addItems(QStringList() << "Asic 1.0" << "Asic 2.0");
    asicBox->setCurrentIndex(Asic2_0);

    Asic_Version = Asic1_0;
    for(auto data : g_algIndexList){
        paramBox->addItem(data.dec);
    }

    connect(asicBox, QOverload<int>::of(&QComboBox::currentIndexChanged), [=](const int &index){
        Asic_Version = static_cast<Asic_Version_e>(index);
        paramBox->clear();
        auto list = Asic_Version == Asic1_0 ? g_algIndexList : g_algIndexList2;
        for(auto data : list){
            paramBox->addItem(data.dec);
        }
        paramBox->setCurrentIndex(0);
    });

    connect(paramBox, &QComboBox::currentTextChanged, this, [=](const QString &content){
        auto list = Asic_Version == Asic1_0 ? g_algIndexList : g_algIndexList2;
        if(content.isEmpty()) return;
        auto it = std::find_if(list.begin(), list.end(), [=](const IspComboInfo &info){
           return info.dec == content;
        });
        curParam = *it;

        QMap<QString,QVariant> map;
        for(auto param : curParam.list){
            map[param.name] = QVariant("");
        }

        tableWidget->setData(map);
    });

    asicBox->setCurrentIndex(Asic_Version);


    auto readBtn = new QPushButton("读数据");
    auto writeBtn = new QPushButton("写数据");
    connect(readBtn, &QPushButton::clicked, this, &ISPParamWidget::readIspParam);

    connect(writeBtn, &QPushButton::clicked, this, &ISPParamWidget::writeIspParam);


    auto hBox1 = new QHBoxLayout;
    auto hBox2 = new QHBoxLayout;
    auto vBox  = new QVBoxLayout;
    hBox1->addWidget(asicBox);
    hBox1->addStretch();
    hBox1->addWidget(paramBox);

    hBox2->addWidget(readBtn);
    hBox2->addStretch();
    hBox2->addWidget(writeBtn);

    vBox->addLayout(hBox1);
    vBox->addWidget(tableWidget);
    vBox->addLayout(hBox2);
    setLayout(vBox);
}

void ISPParamWidget::initConnect()
{

}

void ISPParamWidget::paramParse(QMap<QString, QVariant> &map, const QByteArray &data)
{
    int idx{0};
    if(data.size() <= 0) return;
    const uint8_t *raw = (uint8_t*)data.data();
    std::for_each(curParam.list.begin(), curParam.list.end(), [this,&map,&idx,&raw](const IspItemInfo &info){
        int32_t value{0};
        if(info.word == 1){
            value = raw[idx++];
        }else{
            for(auto i = 0;i < info.word;i++){
                value = (value << 8) | raw[idx++];
            }
        }
        value = info.isUnsigned ? value : -value;
        map[info.name] = QVariant(value);
    });
}

void ISPParamWidget::paramCreate(QMap<QString, QVariant> &map, QByteArray &data)
{
    int idx{0};
    uint8_t *raw = (uint8_t*)data.data();
    std::for_each(curParam.list.begin(), curParam.list.end(), [this,&map,&idx,&raw,&data](const IspItemInfo &info){
        int32_t value = map[info.name].toInt();
        uint8_t temp[8]{0};
        int tempIdx{0};
        if(info.word == 1){
            raw[idx++]  = static_cast<uint8_t>(value);
        }else{
            for(auto i = 0;i < info.word;i++){
                temp[tempIdx++] = (uint8_t)(value & 0xFF);
                value >>= 8;
                idx++;
            }
            std::reverse(temp, temp+tempIdx);
            memcpy(&raw[idx-tempIdx], temp, tempIdx);
        }
    });
}

void ISPParamWidget::readIspParam()
{
    I2CManager i2c;
    QByteArray data;
    QMap<QString,QVariant> map;
    int len{0};
    try{

        qDebug() << __FUNCTION__ << mMedia.toString();
        std::for_each(curParam.list.begin(),curParam.list.end(),[&len](const IspItemInfo &info){
            len += info.word; });
        qDebug() << __FUNCTION__ << mMedia.toString();
        i2c.readIspParam(mMedia.channel, curParam.index, len, data);
        paramParse(map, data);
        tableWidget->setData(map);
    }catch(const std::exception &e){
        qWarning() << e.what();
    }
}

void ISPParamWidget::writeIspParam()
{
    I2CManager i2c;
    QByteArray data;
    QMap<QString,QVariant> map;
    int len{0};
    try{
        std::for_each(curParam.list.begin(),curParam.list.end(),[&len](const IspItemInfo &info){
            len += info.word; });
        data.resize(len);
        tableWidget->getData(map);
        paramCreate(map, data);
        i2c.writeIspParam(mMedia.channel, curParam.index, data);
    } catch (const std::exception &e){
        qWarning() << e.what();
    }
}


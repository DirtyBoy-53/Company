#ifndef ISPPARAMWIDGET_H
#define ISPPARAMWIDGET_H

#include <QWidget>
#include "AlgConst.h"
#include "Media.h"
QT_FORWARD_DECLARE_CLASS(YTableWidget)

class ISPParamWidget : public QWidget
{
public:
    enum Asic_Version_e{
        Asic1_0,
        Asic2_0,
    }Asic_Version;

    ISPParamWidget(QWidget* parent=nullptr);
    void setMedia(Media &media);
    void initUI();
    void initConnect();

private:
    YTableWidget*       tableWidget;
    IspComboInfo        curParam;
    Media               mMedia;

    void paramParse(QMap<QString,QVariant> &map, const QByteArray &data);
    void paramCreate(QMap<QString,QVariant> &map, QByteArray &data);
public slots:
    void readIspParam();
    void writeIspParam();
};

#endif // ISPPARAMWIDGET_H

#include "XyMesDebug.h"
#include "ui_XyMesDebug.h"
#include <QDebug>
#include <QStandardItem>

XyMesDebug::XyMesDebug(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::XyMesDebug)
{
    ui->setupUi(this);
    mMesPtr = new MesCom;

    m_stationName = ConfigInfo::getInstance()->cacheInfo().sCodeId;
    ui->editProcessCode->setText(m_stationName);
    m_orderId = ConfigInfo::getInstance()->cacheInfo().orderId;
    ui->editOrder->setText(m_orderId);
}

XyMesDebug::~XyMesDebug()
{
    delete ui;
}

void XyMesDebug::on_btnEnterMes_clicked()
{
    QString out;
    QString sn = ui->editSN->text();
    ui->textLog->setFont(QFont("Microsoft YaHei UI"));
    ui->textLog->appendPlainText("准备入站，入站SN: " + sn);
    QString processCode = m_stationName;
    if (!ui->editProcessCode->text().isEmpty()) {
        processCode = ui->editProcessCode->text();
    }
    mMesPtr->setUser("gd2222");
    m_userName = mMesPtr->getUser();
    ui->textLog->appendPlainText("当前用户, " + m_userName);
    //mMesPtr->setUser(m_userName);
    if (mMesPtr->enterProduce(sn, out, processCode)) {
        ui->textLog->appendPlainText("入站OK, " + out);
    } else {
        ui->textLog->appendPlainText("入站失败, " + out);
    }
    m_dataPtr.clear();
}


void XyMesDebug::on_btnOuterMes_clicked()
{
    QString out;
    QString sn = ui->editSN->text();
    bool bRst = ui->chkTestResult->isChecked();
    ui->textLog->setFont(QFont("Microsoft YaHei UI"));
    ui->textLog->appendPlainText("准备出站，出站SN: " + sn);

    for (int foo = 0; foo < m_dataPtr.size(); foo++) {
        std::string name = std::get<0>(m_dataPtr.at(foo));
        std::string value = std::get<1>(m_dataPtr.at(foo));
        bool rst = std::get<2>(m_dataPtr.at(foo));
        mMesPtr->addItemResult(QString(name.c_str()), QString(value.c_str()), rst);
        qDebug() << QString(name.c_str()) << QString(value.c_str()) << rst;
    }

    // 0 PASS
    if (mMesPtr->outProduce(bRst ? 0 : 1, out, sn)) {
        ui->textLog->appendPlainText("出站OK, " + out);
    } else {
        ui->textLog->appendPlainText("出站失败, " + out);
    }
}


void XyMesDebug::on_btnBackMes_clicked()
{

}


void XyMesDebug::on_btnGetSN_clicked()
{
    //mMesPtr->getOrderSns()
    QString sns;

    QString orderId = ui->editOrder->text();
    if (orderId.isEmpty()) {
        orderId = ConfigInfo::getInstance()->cacheInfo().orderId;
    }
    mMesPtr->getOrderSns(orderId, sns, ui->checkBox_isPackage->isChecked(), ui->checkBox_isNew->isChecked());
    ui->textLog->appendPlainText(sns);
    ui->editSN->setText(sns);
}


void XyMesDebug::on_btnGetBPList_clicked()
{
    QString sn = ui->editSN->text();
    QStringList tempList;
    QString sensorSn;
    if (mMesPtr->getBadPointInfo(sn, tempList, sensorSn)) {
        ui->textLog->appendPlainText(sensorSn);
        ui->textLog->appendPlainText(tempList.join(" "));
    } else {
        ui->textLog->appendPlainText("获取探测器信息失败");
    }
}


void XyMesDebug::on_btnGetSoft_clicked()
{
    QString path = "D:/update.zip";
    mMesPtr->getVersionFile(path, ui->editSN1->text());
}


void XyMesDebug::on_btnGetSoftVer_clicked()
{

}


void XyMesDebug::on_btnUpload_clicked()
{

}


void XyMesDebug::on_btnGetPrintTemplate_clicked()
{

}


void XyMesDebug::on_btnAddParagraph_clicked()
{
    QString key = ui->editKey->text();
    QString value = ui->editValue->text();
    bool rst = ui->chkResult->isChecked();
    mMesPtr->addItemResult(key, value, rst);
    ui->textLog->appendPlainText(QString("insert data: %1, %2, %3").arg(key).arg(value).arg(rst));
    std::tuple<std::string, std::string, bool> tp = std::make_tuple(key.toStdString(), value.toStdString(), rst);
    m_dataPtr.push_back(tp);
}


void XyMesDebug::on_btnGetSNAppends_clicked()
{
    QList<MaterialInfo> list;
    mMesPtr->getMetrialList(ui->editSN->text(), list);
    qDebug() << "size : " << list.size();
    foreach (MaterialInfo a, list) {
        ui->textLog->appendPlainText(a.basMaterialName + ": " + a.mesBarCode);
    }
}


void XyMesDebug::on_btnGetSNByAppends_clicked()
{
    QString sn;
    mMesPtr->getSnByMaterialSn(ui->editSN->text(), sn);
    ui->textLog->appendPlainText(sn);
}


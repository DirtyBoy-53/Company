#include "XyPropertyPage.h"
#include "ui_XyPropertyPage.h"
#include "CPropertyXml.h"
#include "XyScanDialog.h"
#include <QDate>
#include <QProcess>
#include <QThread>

XyPropertyPage::XyPropertyPage(QDialog *parent) :
    QDialog(parent),
    ui(new Ui::XyPropertyPage)
{
    ui->setupUi(this);
    mPropertyPage = new CPropertyPage(ui->widget);
    this->hide();
    ui->btnAddItem->setEnabled(false);
    ui->btnDelAttr->setEnabled(false);
}

XyPropertyPage::~XyPropertyPage()
{
    delete ui;
}

void XyPropertyPage::showModal(QString path, QString mStationName)
{
    mConfigPath = path;
    mPropertyPage->loadToUI(path, "GLOBAL|" + mStationName);
    this->showNormal();
    ui->btnAddItem->setEnabled(false);
    ui->btnDelAttr->setEnabled(false);
}

void XyPropertyPage::on_pushButton_clicked()
{
    mPropertyPage->saveToXml(mConfigPath);
    emit sigSave();

//    QString path = "cmd.exe /C start cmd /k \"" + QCoreApplication::applicationDirPath() + "/restart.bat" + "\"";
//    system(path.toStdString().c_str());
    this->hide();
}


void XyPropertyPage::on_btnAdminOperate_clicked()
{
    XyScanDialog* scan = new XyScanDialog;
    scan->showModal("输入密码", true);
    QString data = scan->data();
    QDate date = QDate::currentDate();
    int x = date.year() + date.month() + date.day();
    if (data.toInt() == x) {
        ui->btnAddItem->setEnabled(true);
        ui->btnDelAttr->setEnabled(true);
    } else {
        ui->btnAddItem->setEnabled(false);
        ui->btnDelAttr->setEnabled(false);
    }
    delete scan;
    scan = nullptr;
}


void XyPropertyPage::on_btnAddItem_clicked()
{

}


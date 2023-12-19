#include "XyScanDialog.h"
#include "ui_XyScanDialog.h"

XyScanDialog::XyScanDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::XyScanDialog)
{
    ui->setupUi(this);
    ui->editSn->setText("");
    ui->editSn->setFocus();
    mContentData.clear();
    hide();
}

XyScanDialog::~XyScanDialog()
{
    delete ui;
}

QString XyScanDialog::data()
{
    return mContentData;
}

void XyScanDialog::showModal(QString labelName, bool passwd)
{
    ui->label->setText(labelName);
    ui->editSn->setText("");
    ui->editSn->setFocus();
    if (passwd) {
        ui->editSn->setEchoMode(QLineEdit::Password);
    } else {
        ui->editSn->setEchoMode(QLineEdit::Normal);
    }
    mContentData.clear();
    mContentList.clear();
    this->exec();
}

void XyScanDialog::on_editSn_returnPressed()
{
    mContentList << "OK";
    mContentList << ui->editSn->text();
    mContentData = ui->editSn->text();
    this->hide();
}


void XyScanDialog::on_btnOk_clicked()
{
    on_editSn_returnPressed();
}


void XyScanDialog::on_btnCancel_clicked()
{
    mContentList << "NG";
    // this->hide();
    close();
}


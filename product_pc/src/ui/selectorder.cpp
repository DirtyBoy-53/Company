#include "selectorder.h"
#include "ui_selectorder.h"
#include <configinfo.h>
#include <mesclient.h>
#include <QMessageBox>

SelectOrder::SelectOrder(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::SelectOrder)
{
    ui->setupUi(this);
    setWindowTitle("工单设置");
}

SelectOrder::~SelectOrder()
{
    delete ui;
}

void SelectOrder::on_pushButton_clicked()
{
    CacheInfo info = ConfigInfo::getInstance()->cacheInfo();
    info.orderId = ui->lineEdit->text();
    ConfigInfo::getInstance()->setCacheInfo(info);
    close();
}


void SelectOrder::on_comboBox_currentIndexChanged(int index)
{
    ui->lineEdit->setText(ui->comboBox->currentText());
}

void SelectOrder::showEvent(QShowEvent *event)
{
    m_list.clear();
    CacheInfo info = ConfigInfo::getInstance()->cacheInfo();
    ui->lineEdit->setText(info.orderId);
    int iRet = MesClient::instance()->getOrderIds(m_list);
    if(iRet == 0) {
        ui->comboBox->clear();
        for(int i = 0; i < m_list.size(); i ++) {
            ui->comboBox->addItem(m_list.at(i).key);
        }
    }
}


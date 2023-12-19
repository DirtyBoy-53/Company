#include "screwcfgpage.h"
#include "ui_screwcfgpage.h"
#include <QDebug>
#include <QStandardItemModel>
#include <QMessageBox>
#include <QFileDialog>
#include "configinfo.h"
#include "configinfoelec.h"
#include "StaticSignals.h"

ScrewCfgPage::ScrewCfgPage(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::ScrewCfgPage)
{
    ui->setupUi(this);
    m_model = new QStandardItemModel();
    m_modelTips = new QStandardItemModel();

//    initSysTableWidget();
//    initTableWidget();
//    initTableTips();
//    initCfgTableWidget();
//    initWorkListWidget();
    connect(ui->screwListWidget,SIGNAL(clicked(QModelIndex)),this,SLOT(clickedListView(QModelIndex)));
    connect(StaticSignals::getInstance(), &StaticSignals::statusUpdatePos, this, &ScrewCfgPage::slotUpdateState);
}

ScrewCfgPage::~ScrewCfgPage()
{
    delete ui;
}

void ScrewCfgPage::showInit()
{
    initSysTableWidget();
    initTableWidget();
    initTableTips();
    initCfgTableWidget();
    initWorkListWidget();
    show();
}

void ScrewCfgPage::initTableWidget()
{
    m_model->clear();

    /*设置列字段名*/
    m_model->setColumnCount(7);
    m_model->setHeaderData(0,Qt::Horizontal, "螺丝编号");
    m_model->setHeaderData(1,Qt::Horizontal, "螺丝配置");
    m_model->setHeaderData(2,Qt::Horizontal, "X坐标");
    m_model->setHeaderData(3,Qt::Horizontal, "Y坐标");
    m_model->setHeaderData(4,Qt::Horizontal, "偏差范围");
    m_model->setHeaderData(5,Qt::Horizontal, "图片名称");
    m_model->setHeaderData(6,Qt::Horizontal, "批头编号");
    ui->tableView->setModel(m_model);

    ui->tableView->setShowGrid(true);
    ui->tableView->setSelectionBehavior(QAbstractItemView::SelectRows);
    ui->tableView->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);

    QList<LockPosInfo> infoList;
    ConfigInfoElecLock::getInstance()->GetLockInfo(infoList);
    for(int i=0; i<infoList.size() ;i++) {
        LockPosInfo info = infoList[i];
        QList<QStandardItem*> list;
        list << new QStandardItem(info.name)
             << new QStandardItem(info.cfgIndex)
             << new QStandardItem(info.x)
             << new QStandardItem(info.y)
             << new QStandardItem(info.limit)
             << new QStandardItem(info.imgPath)
             << new QStandardItem(info.screwModel);
        m_model->insertRow(m_model->rowCount(), list);
    }
}

void ScrewCfgPage::initTableTips()
{
    m_modelTips->clear();
    /*设置列字段名*/
    m_modelTips->setColumnCount(4);
    m_modelTips->setHeaderData(0,Qt::Horizontal, "提示编号");
    m_modelTips->setHeaderData(1,Qt::Horizontal, "标题文字");
    m_modelTips->setHeaderData(2,Qt::Horizontal, "弹框文字");
    m_modelTips->setHeaderData(3,Qt::Horizontal, "图片名称");
    ui->tipTableView->setModel(m_modelTips);

    ui->tipTableView->setShowGrid(true);
    ui->tipTableView->setSelectionBehavior(QAbstractItemView::SelectRows);
    ui->tipTableView->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);

    QList<CustomWorkList> tipsList;
    ConfigInfoElecLock::getInstance()->GetTipsList(tipsList);
    for(int i=0; i<tipsList.size() ;i++) {
        CustomWorkList info = tipsList[i];
        QList<QStandardItem*> list;
        list << new QStandardItem(info.index) << new QStandardItem(info.txt) << new QStandardItem(info.tips) << new QStandardItem(info.path);
        m_modelTips->insertRow(m_modelTips->rowCount(), list);
    }
}

void ScrewCfgPage::initCfgTableWidget()
{
    LockCfgInfo cfgInfo;
    ConfigInfoElecLock::getInstance()->GetLockCfgInfo(0, cfgInfo);
    ui->tableWidget->setItem(0, 1, new QTableWidgetItem(cfgInfo.mnm));
    ui->tableWidget->setItem(1, 1, new QTableWidgetItem(cfgInfo.mnmLimit));
    ui->tableWidget->setItem(2, 1, new QTableWidgetItem(cfgInfo.step1Circle));
    ui->tableWidget->setItem(3, 1, new QTableWidgetItem(cfgInfo.step1Speed));
    ui->tableWidget->setItem(4, 1, new QTableWidgetItem(cfgInfo.step2Circle));
    ui->tableWidget->setItem(5, 1, new QTableWidgetItem(cfgInfo.step2Speed));
    ui->tableWidget->setItem(6, 1, new QTableWidgetItem(cfgInfo.step3Circle));
    ui->tableWidget->setItem(7, 1, new QTableWidgetItem(cfgInfo.step3Speed));
    ui->tableWidget->setItem(8, 1, new QTableWidgetItem(cfgInfo.circleFugao));
    ui->tableWidget->setItem(9, 1, new QTableWidgetItem(cfgInfo.circleHuaya));
}

void ScrewCfgPage::initSysTableWidget()
{
    QString imgPath;
    ConfigInfoElecLock::getInstance()->GetImagePath(imgPath);
    ui->imgPath->setText(imgPath);

    SystemInfo sysInfo;
    ConfigInfoElecLock::getInstance()->GetSysInfo(sysInfo);
    ui->editScrewCom->setText(sysInfo.writeCom);
    ui->editSupportCom->setText(sysInfo.readCom);
    ui->editSelectCom->setText(sysInfo.selectCom);
    ui->editIoCom->setText(sysInfo.ioCom);
    ui->checkBoxSelect->setChecked(sysInfo.selectSwitch);
    ui->checkBoxIo->setChecked(sysInfo.ioSwitch);
}

void ScrewCfgPage::initWorkListWidget()
{
    ui->listWidgetWork->clear();

    QList<AutoWorkIndex> workList;
    ConfigInfoElecLock::getInstance()->GetWorkList(workList);
    for(int i=0; i<workList.size(); i++) {
        AutoWorkIndex info = workList[i];
        if(info.type == 0) {
            ui->listWidgetWork->addItem(QString("螺丝: %1").arg(info.index));
        } else if(info.type == 1) {
            ui->listWidgetWork->addItem(QString("提示: %1").arg(info.index));
        }
    }
    qDebug() << "initWorkListWidget2 is " << ui->listWidgetWork->currentRow();
}

void ScrewCfgPage::slotUpdateState(const int x, const int y)
{
    m_x = x;
    m_y = y;
}

void ScrewCfgPage::on_btnAdd_clicked()
{
    if(m_x == 0 && m_y == 0) {
        QMessageBox::information(this, "tips", "添加失败,请先点击检查设备环境按钮");
        return ;
    }

    int index = ui->screwListWidget->currentIndex().row();
    if(index <= 0) index = 0;
    QList<QStandardItem*> list;
    list << new QStandardItem(QString::number(m_model->rowCount()+1))
         << new QStandardItem(QString("配置%1").arg(index+1))
         << new QStandardItem(QString::number(m_x)) << new QStandardItem(QString::number(m_y))
         << new QStandardItem(QString::number(300))
         << new QStandardItem(QString("1_%1.png").arg(m_model->rowCount()+1))
         << new QStandardItem(QString::number(1));
    m_model->insertRow(m_model->rowCount(), list);
}

void ScrewCfgPage::on_btnDelete_clicked()
{
    QModelIndex index = ui->tableView->currentIndex();
    m_model->removeRow(index.row());
}

void ScrewCfgPage::on_btnSaveCfgInfo_clicked()
{
    int index = ui->screwListWidget->currentIndex().row();
    if(index <= 0) index = 0;

    LockCfgInfo cfgInfo;
    ConfigInfoElecLock::getInstance()->GetLockCfgInfo(index, cfgInfo);
    cfgInfo.mnm = ui->tableWidget->item(0, 1)->text();
    cfgInfo.mnmLimit = ui->tableWidget->item(1, 1)->text();
    cfgInfo.step1Circle = ui->tableWidget->item(2, 1)->text();
    cfgInfo.step1Speed = ui->tableWidget->item(3, 1)->text();
    cfgInfo.step2Circle = ui->tableWidget->item(4, 1)->text();
    cfgInfo.step2Speed = ui->tableWidget->item(5, 1)->text();
    cfgInfo.step3Circle = ui->tableWidget->item(6, 1)->text();
    cfgInfo.step3Speed = ui->tableWidget->item(7, 1)->text();
    cfgInfo.circleFugao = ui->tableWidget->item(8, 1)->text();
    cfgInfo.circleHuaya = ui->tableWidget->item(9, 1)->text();
    ConfigInfoElecLock::getInstance()->SetLockCfgInfo(index, cfgInfo);

    QMessageBox::information(this,"提示","配置保存成功");
}

void ScrewCfgPage::on_btnDeleteBefore_clicked()
{
    QModelIndex index = ui->tipTableView->currentIndex();
    m_modelTips->removeRow(index.row());
}

void ScrewCfgPage::on_btnAddBefore_clicked()
{
    QList<QStandardItem*> list;
    list << new QStandardItem(QString("提示%1").arg(m_modelTips->rowCount()+1))
         << new QStandardItem(QString("双击填写提示文字").arg(m_modelTips->rowCount()+1))
         << new QStandardItem("双击填写弹框文字")
         << new QStandardItem(QString("0_%1.png").arg(m_modelTips->rowCount()+1));
    m_modelTips->insertRow(m_modelTips->rowCount(), list);
}

void ScrewCfgPage::on_pushButton_clicked()
{
    QString filePath = QFileDialog::getExistingDirectory();
    if(filePath.isEmpty()) {
        return;
    }
    ui->imgPath->setText(filePath);
    ConfigInfoElecLock::getInstance()->SetImagePath(filePath);
}

void ScrewCfgPage::on_btnConfirm_clicked()
{
    QList<LockPosInfo> list;
    LockPosInfo info;
    for(int i=0; i<m_model->rowCount(); i++) {
        info.name = m_model->item(i, 0)->text();
        info.cfgIndex = m_model->item(i, 1)->text();
        info.x = m_model->item(i, 2)->text();
        info.y = m_model->item(i, 3)->text();
        info.limit = m_model->item(i, 4)->text();
        info.imgPath = m_model->item(i, 5)->text();
        info.screwModel = m_model->item(i, 6)->text();
        list << info;
        qDebug() << info.cfgIndex << " " << info.name;
    }
    ConfigInfoElecLock::getInstance()->SetLockInfo(list);

    QList<CustomWorkList> tipsList;
    for(int i=0; i<m_modelTips->rowCount(); i++) {
        CustomWorkList info;
        info.index = m_modelTips->item(i, 0)->text();
        info.txt = m_modelTips->item(i, 1)->text();
        info.tips = m_modelTips->item(i, 2)->text();
        info.path = m_modelTips->item(i, 3)->text();
        tipsList << info;
        qDebug() << info.index << " " << info.txt;
    }
    ConfigInfoElecLock::getInstance()->SetTipsList(tipsList);

    QList<AutoWorkIndex> workList;
    int count = ui->listWidgetWork->count();

    for(int i=0; i<count; i++) {
        AutoWorkIndex info;
        QString value = ui->listWidgetWork->item(i)->text();
        if(value.contains("螺丝")) {
            info.type = 0;
        } else if(value.contains("提示")) {
            info.type = 1;
        } else {
            continue;
        }
        info.index = value.right(1).toInt();
        workList << info;
        qDebug() << info.type << " " << info.index;
    }
    ConfigInfoElecLock::getInstance()->SetWorkList(workList);

    SystemInfo sysInfo;
    ConfigInfoElecLock::getInstance()->GetSysInfo(sysInfo);
    sysInfo.writeCom = ui->editScrewCom->text();
    sysInfo.readCom = ui->editSupportCom->text();
    sysInfo.selectCom = ui->editSelectCom->text();
    sysInfo.selectSwitch = ui->checkBoxSelect->isChecked();
    sysInfo.ioCom = ui->editIoCom->text();
    sysInfo.ioSwitch = ui->checkBoxIo->isChecked();
    ConfigInfoElecLock::getInstance()->SetSysInfo(sysInfo);

    this->hide();
}

void ScrewCfgPage::clickedListView(QModelIndex model)
{
    int index = model.row();
    LockCfgInfo cfgInfo;
    ConfigInfoElecLock::getInstance()->GetLockCfgInfo(index, cfgInfo);
    ui->tableWidget->item(0, 1)->setText(cfgInfo.mnm);
    ui->tableWidget->item(1, 1)->setText(cfgInfo.mnmLimit);
    ui->tableWidget->item(2, 1)->setText(cfgInfo.step1Circle);
    ui->tableWidget->item(3, 1)->setText(cfgInfo.step1Speed);
    ui->tableWidget->item(4, 1)->setText(cfgInfo.step2Circle);
    ui->tableWidget->item(5, 1)->setText(cfgInfo.step2Speed);
    ui->tableWidget->item(6, 1)->setText(cfgInfo.step3Circle);
    ui->tableWidget->item(7, 1)->setText(cfgInfo.step3Speed);
    ui->tableWidget->item(8, 1)->setText(cfgInfo.circleFugao);
    ui->tableWidget->item(9, 1)->setText(cfgInfo.circleHuaya);
}

void ScrewCfgPage::on_btnCancle_clicked()
{
    this->hide();
}

void ScrewCfgPage::on_btnInsertScrew_clicked()
{
    int index = ui->tableView->currentIndex().row();
    if(index <= 0) index = 0;

    int currentRow = ui->listWidgetWork->currentRow();
    if(currentRow != -1) {
        ui->listWidgetWork->insertItem(currentRow+1, QString("螺丝: %1").arg(index+1));
        ui->listWidgetWork->setCurrentRow(currentRow+1);
    } else {
        ui->listWidgetWork->addItem(QString("螺丝: %1").arg(index+1));
    }
}

void ScrewCfgPage::on_btnInsertTips_clicked()
{
    int index = ui->tipTableView->currentIndex().row();
    if(index <= 0) index = 0;

    int currentRow = ui->listWidgetWork->currentRow();
    if(currentRow != -1) {
        ui->listWidgetWork->insertItem(currentRow+1, QString("提示: %1").arg(index+1));
        ui->listWidgetWork->setCurrentRow(currentRow+1);
    } else {
        ui->listWidgetWork->addItem(QString("提示: %1").arg(index+1));
    }
}


void ScrewCfgPage::on_btnClearList_clicked()
{
    ui->listWidgetWork->clear();

    QList<AutoWorkIndex> workList;
    ConfigInfoElecLock::getInstance()->SetWorkList(workList);
}


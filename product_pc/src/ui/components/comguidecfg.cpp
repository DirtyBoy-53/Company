#include "comguidecfg.h"
#include "ui_comguidecfg.h"
#include <QStandardItem>
#include <QDebug>
#include <QSettings>
#include <QTextCodec>
#include <QMessageBox>
#include "configinfo.h"
#include <QFileDialog>

QList<GuideCfgInfo> ComGuideCfg::m_list;

ComGuideCfg::ComGuideCfg(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::ComGuideCfg)
{
    ui->setupUi(this);
    m_produceName = ConfigInfo::getInstance()->getValueString("项目选择");
    m_configPath = QCoreApplication::applicationDirPath() + QString("/config/guidecfg_%1.ini").arg(m_produceName);
    initWidget();
}

ComGuideCfg::~ComGuideCfg()
{
    delete ui;
}

void ComGuideCfg::changeProduce(const QString& produce)
{
    m_produceName = ConfigInfo::getInstance()->getValueString("项目选择");
    m_configPath = QCoreApplication::applicationDirPath() + QString("/config/guidecfg_%1.ini").arg(m_produceName);
    initWidget();
}

void ComGuideCfg::getGuideCfgList(QList<GuideCfgInfo>& list)
{
    list = m_list;
}

void ComGuideCfg::initWidget()
{
    ui->tableWidget->clear();
    int rowCount = ui->tableWidget->rowCount();
    for(int i=rowCount; i>=0; i--) {
        ui->tableWidget->removeRow(i);
    }

    QStringList head;
    head << "标题文字" << "弹框文字" << "图片名称";
    ui->tableWidget->setHorizontalHeaderLabels(head);
    ui->tableWidget->setShowGrid(true);
    ui->tableWidget->setSelectionBehavior(QAbstractItemView::SelectRows);
    ui->tableWidget->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);

    m_list.clear();
    QSettings reader(m_configPath, QSettings::IniFormat);
    reader.setIniCodec(QTextCodec::codecForName("UTF-8"));
    reader.beginGroup("config");

    QString defaultPath = QCoreApplication::applicationDirPath() + "/res/";
    m_imgPath = reader.value("imgPath", QVariant(defaultPath)).toString();
    ui->lineEdit->setText(m_imgPath);

    QStringList keyList = reader.allKeys();
    for(int i=0; i<keyList.size(); i++) {
        QStringList list = reader.value(keyList[i], QVariant("0")).toString().split("~");
        if(list.size() != 3) break;

        GuideCfgInfo info;
        info.tips = list[0];
        info.msgTips = list[1];
        QString imgName = list[2];
        info.path = m_imgPath + "/" + imgName;
        m_list.append(info);

        ui->tableWidget->insertRow(i);
        ui->tableWidget->setItem(i, 0, new QTableWidgetItem(info.tips));
        ui->tableWidget->setItem(i, 1, new QTableWidgetItem(info.msgTips));
        ui->tableWidget->setItem(i, 2, new QTableWidgetItem(imgName));
    }
    reader.endGroup();
}

void ComGuideCfg::on_btnDelete_clicked()
{
    QModelIndex index = ui->tableWidget->currentIndex();
    ui->tableWidget->removeRow(index.row());
}

void ComGuideCfg::on_btnAdd_clicked()
{
    int rowCount = ui->tableWidget->rowCount();
    ui->tableWidget->insertRow(rowCount);
    ui->tableWidget->setItem(rowCount, 0, new QTableWidgetItem("双击填写提示文字"));
    ui->tableWidget->setItem(rowCount, 1, new QTableWidgetItem("双击填写弹框文字"));
    ui->tableWidget->setItem(rowCount, 2, new QTableWidgetItem(QString("%1.png").arg(rowCount+1)));
}

void ComGuideCfg::on_btnSave_clicked()
{
    m_list.clear();

    QSettings writer(m_configPath, QSettings::IniFormat);
    writer.setIniCodec(QTextCodec::codecForName("UTF-8"));
    writer.remove("config");
    writer.beginGroup("config");

    QString path = ui->lineEdit->text();
    writer.setValue("imgPath", path);

    for(int i=0; i<ui->tableWidget->rowCount(); i++) {
        GuideCfgInfo info;
        info.tips = ui->tableWidget->item(i, 0)->text();
        info.msgTips = ui->tableWidget->item(i, 1)->text();
        QString imgName = ui->tableWidget->item(i, 2)->text();
        info.path = path + "/" + imgName;

        m_list.append(info);

        QString value = QString("%1~%2~%3").arg(info.tips).arg(info.msgTips).arg(imgName);
        writer.setValue(QString("guide%1").arg(i), value);
        qDebug() << "insert row " << i;
    }


    writer.endGroup();
    QMessageBox::information(this, "tips", "保存成功");
}

void ComGuideCfg::on_pushButton_clicked()
{
    QString filePath = QFileDialog::getExistingDirectory();
    if(filePath.isEmpty()) {
        return;
    }
    ui->lineEdit->setText(filePath);
}


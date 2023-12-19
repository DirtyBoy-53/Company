#include "lifecontrolsetup.h"
#include <QDebug>
#include <QSettings>
#include <QTextCodec>
#include "ui_lifecontrolsetup.h"
#include <QMessageBox>
#include <QFileDialog>
#include "doubleclicklabel.h"

LifeControlSetup::LifeControlSetup(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::LifeControlSetup)
{
    ui->setupUi(this);
    m_itemModel = new QStandardItemModel();
    ui->tableView->setModel(m_itemModel);

    QStringList header;
    header << "名称" << "料号" << "设计寿命次数" << "已使用次数" << "已使用百分比" << "更换" << "寿命调整" << "图片路径";
    m_itemModel->setHorizontalHeaderLabels(header);
    // ui->tableView->horizontalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);
    ui->tableView->horizontalHeader()->setStretchLastSection(true);
    ui->tableView->setSelectionMode(QAbstractItemView::SingleSelection);
    ui->tableView->setSelectionBehavior(QAbstractItemView::SelectRows);

    connect(m_itemModel, &QStandardItemModel::itemChanged, this, &LifeControlSetup::on_itemChanged);
}

LifeControlSetup::~LifeControlSetup()
{
    delete ui;
}

void LifeControlSetup::show(bool isAdmin)
{
    if (!isAdmin) {
        ui->widgetLayout->setEnabled(false);
    } else {
        ui->widgetLayout->setEnabled(true);
    }

    QString path = QCoreApplication::applicationDirPath() + "/config/" + "lifeSetup.ini";
    QSettings setting(path, QSettings::IniFormat);
    QTextCodec *codec = QTextCodec::codecForName("GBK");
    setting.setIniCodec(codec);
    qDebug() << path;

    while (m_itemModel->rowCount() > 0) {
        m_itemModel->removeRow(0);
    }
    // m_itemModel->clear(); 会清除头部信息

    QStringList groups = setting.childGroups();
    foreach (QString grp, groups) {
        setting.beginGroup(grp);
        CustomProperty cp;
        cp.name = setting.value("name").toString();
        cp.code = setting.value("code").toString();
        cp.totalTimes = setting.value("total").toInt();
        cp.usedTimes = setting.value("used").toInt();
        cp.addedTimes = setting.value("added").toInt();
        cp.imgPath = setting.value("path").toString();
        setting.endGroup();

        addRowData(cp);
    }
    int rows = m_itemModel->rowCount();

    qDebug() << rows;


    showNormal();
}

void LifeControlSetup::showEvent(QShowEvent *event)
{
    QSize s = ui->tableView->size();
    qDebug() << s;

    //ui->tableView->setColumnWidth(0, 50);
    //ui->tableView->setColumnWidth(1, s.width()/3);
}

// header:
// "名称" << "料号" << "设计寿命次数" << "已使用次数" << "已使用百分比" << "更换" << "寿命调整" << "图片路径"
void LifeControlSetup::addRowData(CustomProperty cp)
{
    int row = m_itemModel->rowCount();

    QList<QStandardItem*> list;
    // name ...
    QStandardItem* item = new QStandardItem();
    item->setText(cp.name);
    item->setTextAlignment(Qt::AlignCenter | Qt::AlignHCenter);
    list << item;

    // 料号
    item = new QStandardItem();
    item->setText(cp.code);
    item->setTextAlignment(Qt::AlignCenter | Qt::AlignHCenter);
    list << item;

    // 寿命总次数
    item = new QStandardItem();
    item->setText(QString::number(cp.totalTimes));
    item->setTextAlignment(Qt::AlignCenter | Qt::AlignHCenter);
    list << item;

    // 已使用次数
    item = new QStandardItem();
    item->setText(QString::number(cp.usedTimes));
    item->setTextAlignment(Qt::AlignCenter | Qt::AlignHCenter);
    item->setEditable(false);
    list << item;

    int plus = cp.totalTimes + cp.addedTimes;
    if (plus == 0) {
        plus = 1;
    }
    // 已使用百分比
    float rate = 100 * (float(cp.usedTimes ) / float(plus));
    item = new QStandardItem();
    item->setText(QString("%1%").arg(QString::number(rate, 'f', 2)));
    item->setTextAlignment(Qt::AlignCenter | Qt::AlignHCenter);
    item->setEditable(false);
    if (rate >= 100) {
        item->setBackground(QBrush(QColor(220, 0, 0)));
    } else if(rate > 95) {
        item->setBackground(QBrush(QColor(250, 250, 0)));
    } else {
        item->setBackground(QBrush(QColor(0, 220, 0)));
    }
    list << item;

    // 更换
    item = new QStandardItem();
    list << item;

    // 寿命调整
    item = new QStandardItem();
    item->setText(QString::number(cp.addedTimes));
    item->setTextAlignment(Qt::AlignCenter | Qt::AlignHCenter);
    list << item;

    // 图片路径
    item = new QStandardItem();
    list << item;

    m_itemModel->insertRow(row, list);

    // 增加修改按钮功能
    QPushButton * btn = new QPushButton(this);
    btn->setText("更换");
    btn->setProperty("row", QVariant(row));
    connect(btn, &QPushButton::clicked, this, [this](){
        if (QMessageBox::Ok == QMessageBox::information(this, "提示", "确定执行更换耗材操作?", QMessageBox::Ok | QMessageBox::No)) {
            QPushButton* b = (QPushButton*)sender();
            int row = b->property("row").toInt();
            // qDebug() << row ;
            m_itemModel->setData(m_itemModel->index(row, 3), QVariant(0));
            m_itemModel->setData(m_itemModel->index(row, 4), QVariant(0));
            m_itemModel->setData(m_itemModel->index(row, 6), QVariant(0));
            saveData();
        }
    });
    ui->tableView->setIndexWidget(m_itemModel->index(row, 5), btn);

    // 增加显示按钮功能
    DoubleClickLabel * label = new DoubleClickLabel();
    label->setText("双击选择图片");
    if (!cp.imgPath.isEmpty()) {
        label->setText(cp.imgPath);
        m_itemModel->setData(m_itemModel->index(row, 7), QVariant(cp.imgPath));
    }
    label->setProperty("row", QVariant(row));
    connect(label, &DoubleClickLabel::sigDoubleClicked, this, [this](){
        DoubleClickLabel* label = (DoubleClickLabel*)sender();
        QString path = QFileDialog::getOpenFileName(this, "选择图片文件", "./", "All files(*.*);;Images (*.jpg *.jpeg *.bmp *.png)");
        if (QFile::exists(path)) {
            label->setText(path);
            label->setToolTip(path);
            int row = label->property("row").toInt();
            m_itemModel->setData(m_itemModel->index(row, 7), QVariant(path));
        }
    });
    ui->tableView->setIndexWidget(m_itemModel->index(row, 7), label);
}

void LifeControlSetup::removeRowData()
{
    QModelIndex idx = ui->tableView->selectionModel()->currentIndex();
    m_itemModel->removeRow(idx.row());
}

void LifeControlSetup::saveData()
{
    QString path = QCoreApplication::applicationDirPath() + "/config/" + "lifeSetup.ini";
    QSettings setting(path, QSettings::IniFormat);
    QTextCodec *codec = QTextCodec::codecForName("GBK");
    setting.setIniCodec(codec);
    qDebug() << path;
    FILE* p = fopen(path.toStdString().c_str(), "w");
    if (p) {
        fclose(p);
    }

    int rows = m_itemModel->rowCount();
    qDebug() << rows;

    float max_rate = .0f;
    for (int foo = 0; foo < rows; ++foo) {
        int total = m_itemModel->item(foo, 2)->text().toInt();
        int used = m_itemModel->item(foo, 3)->text().toInt();
        int added = m_itemModel->item(foo, 6)->text().toInt();
        setting.beginGroup(QString("KEY%1").arg(foo));
        setting.setValue("name", QVariant(m_itemModel->item(foo, 0)->text()));
        setting.setValue("code", QVariant(m_itemModel->item(foo, 1)->text()));
        setting.setValue("total", QVariant(total));
        setting.setValue("used", QVariant(used));
        setting.setValue("added", QVariant(added));
        setting.setValue("path", QVariant(m_itemModel->item(foo, 7)->text()));
        setting.endGroup();


        int plus = total + added;
        if (plus == 0) {
            plus = 1;
        }

        // 已使用百分比
        float rate = 100 * (float(used) / float(plus));
        QStandardItem* item = m_itemModel->itemFromIndex(m_itemModel->index(foo, 4));
        item->setText(QString("%1%").arg(QString::number(rate, 'f', 2)));
        if (rate >= 100) {
            item->setBackground(QBrush(QColor(220, 0, 0)));
        } else if(rate > 95) {
            item->setBackground(QBrush(QColor(250, 250, 0)));
        } else {
            item->setBackground(QBrush(QColor(0, 220, 0)));
        }
        if (rate > max_rate) {
            max_rate = rate;
        }
    }

    if (max_rate < 95) {
        emit sigResetLabelStatus();
    }
}

//void LifeControlSetup::slotUpdateUsed()
//{
//    QString path = QCoreApplication::applicationDirPath() + "/config/" + "lifeSetup.ini";
//    QSettings setting(path, QSettings::IniFormat);
//    QTextCodec *codec = QTextCodec::codecForName("GBK");
//    setting.setIniCodec(codec);
//    qDebug() << path;
//    FILE* p = fopen(path.toStdString().c_str(), "w");
//    if (p) {
//        fclose(p);
//    }
//    int rows = m_itemModel->rowCount();
//    qDebug() << rows;

//    for (int foo = 0; foo < rows; ++foo) {
//        int used = m_itemModel->item(foo, 3)->text().toInt();
//        int total = m_itemModel->item(foo, 2)->text().toInt();
//        int added = m_itemModel->item(foo, 6)->text().toInt();
//        used++;

//        setting.beginGroup(QString("KEY%1").arg(foo));
//        setting.setValue("name", QVariant(m_itemModel->item(foo, 0)->text()));
//        setting.setValue("code", QVariant(m_itemModel->item(foo, 1)->text()));
//        setting.setValue("total", QVariant(total));
//        setting.setValue("used", QVariant(used));
//        setting.setValue("added", QVariant(added));
//        setting.endGroup();


//        int plus = total + added;
//        if (plus == 0) {
//            plus = 1;
//        }

//        // 已使用百分比
//        float rate = 100 * (float(used) / float(plus));
//        QStandardItem* item = m_itemModel->itemFromIndex(m_itemModel->index(foo, 4));
//        item->setText(QString("%1%").arg(QString::number(rate, 'f', 2)));
//        if (rate >= 100) {
//            emit sigOutLimitation();
//            item->setBackground(QBrush(QColor(220, 0, 0)));
//        } else if (rate > 98) {
//            item->setBackground(QBrush(QColor(220, 0, 0)));
//        } else if(rate > 95) {
//            item->setBackground(QBrush(QColor(250, 250, 0)));
//        } else {
//            item->setBackground(QBrush(QColor(0, 220, 0)));
//        }
//    }
//}


void LifeControlSetup::on_btnPlus_clicked()
{
    CustomProperty cp;
    cp.name = "耗材";
    cp.code = "AAAAA";
    cp.totalTimes = 10000;
    cp.usedTimes = 0;
    cp.addedTimes = 0;
    addRowData(cp);
}


void LifeControlSetup::on_btnMunus_clicked()
{
    if (QMessageBox::Ok == QMessageBox::information(this, "提示", "确定删除所选行?", QMessageBox::Ok | QMessageBox::No)) {
        removeRowData();
    }
}

// SAVE
void LifeControlSetup::on_pushButton_clicked()
{
    if (QMessageBox::Ok == QMessageBox::information(this, "提示", "是否保存当前页面数据?", QMessageBox::Ok | QMessageBox::No)) {
        saveData();
        // close();
        QMessageBox::information(this, "提示", "数据保存完成", QMessageBox::Ok | QMessageBox::No);
    }
}

void LifeControlSetup::on_itemChanged(QStandardItem *item)
{
    QModelIndex index = m_itemModel->indexFromItem(item);
    int foo = index.row();
    int total = m_itemModel->item(foo, 2)->text().toInt();
    int used = m_itemModel->item(foo, 3)->text().toInt();
    int added = m_itemModel->item(foo, 6)->text().toInt();

    int plus = total + added;
    if (plus == 0) {
        plus = 1;
    }

    // 已使用百分比
    float rate = 100 * (float(used) / float(plus));
    QStandardItem* itemX = m_itemModel->itemFromIndex(m_itemModel->index(foo, 4));
    itemX->setText(QString("%1%").arg(QString::number(rate, 'f', 2)));
    if (rate >= 100) {
        itemX->setBackground(QBrush(QColor(220, 0, 0)));
    } else if(rate > 95) {
        itemX->setBackground(QBrush(QColor(250, 250, 0)));
    } else {
        itemX->setBackground(QBrush(QColor(0, 220, 0)));
    }
}


void LifeControlSetup::on_pushButton_2_clicked()
{
    close();
}


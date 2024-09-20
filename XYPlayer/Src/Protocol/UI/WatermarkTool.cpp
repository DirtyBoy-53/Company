#include "WatermarkTool.h"

#include <QStandardItemModel>
#include <QComboBox>
#include <QMap>
#include <QDebug>
#include <QPushButton>
#include <QColorDialog>
#include "YFunction.h"

void QComboBox::wheelEvent(QWheelEvent *e)
{
    //啥也不干，屏蔽鼠标滚动
}

WatermarkTool::WatermarkTool(QWidget *parent)
    : QWidget(parent)
{
    initUI();
    initConnect();
}

WatermarkTool::~WatermarkTool()
{

}

void WatermarkTool::initUI()
{
    setFixedHeight(400);
    mTableWidget = new QTableWidget();
    // mTableWidget->setStyleSheet("background-color:lightblue;");

    QStringList headList;
    headList << "X坐标" << "Y坐标" << "颜色预览" << "颜色选择" << "文字大小" << "文字内容";
    mTableWidget->setColumnCount(headList.size());

    for(int i=0; i<headList.size(); i++) {
        mTableWidget->setHorizontalHeaderItem(i,new QTableWidgetItem(headList.at(i)));
    }
    mTableWidget->horizontalHeader()->setStretchLastSection(true);
    mTableWidget->setSelectionBehavior(QAbstractItemView::SelectRows);
    mTableWidget->verticalHeader()->setVisible(false); //设置垂直头不可见

    auto btnAdd = new QPushButton("添加");
    auto btnDel = new QPushButton("删除");
    auto btnOK = new QPushButton("确定");
    connect(btnOK, &QPushButton::clicked, this, &WatermarkTool::on_btnConfirm_clicked);
    connect(btnAdd, &QPushButton::clicked, this, &WatermarkTool::on_btnAdd_clicked);
    connect(btnDel, &QPushButton::clicked, this, &WatermarkTool::on_btnDelete_clicked);


    auto layout = new QVBoxLayout;
    auto hbox = new QHBoxLayout;
    hbox->addWidget(btnAdd);
    hbox->addWidget(btnDel);
    layout->addWidget(mTableWidget);
    layout->addLayout(hbox);
    layout->addWidget(btnOK);
    setLayout(layout);

}

void WatermarkTool::initConnect()
{

}

void applyColor(const QColor &color, QLabel *label){
    QString colorStr = QString("background-color:%1;").arg(color.name(QColor::HexRgb));
    label->setStyleSheet(colorStr);
    label->setText(color.name(QColor::HexRgb).toUpper());
}

void WatermarkTool::on_btnAdd_clicked()
{
    int curRow = mTableWidget->rowCount();
    mTableWidget->insertRow(curRow);
    mTableWidget->setItem(curRow, 0, new QTableWidgetItem("0"));
    mTableWidget->setItem(curRow, 1, new QTableWidgetItem("0"));

    auto label = new QLabel;
    applyColor(Qt::red, label);
    mTableWidget->setCellWidget(curRow, 2, label);

    QPushButton *btnColor = genPushButton(QPixmap(":color"), "选择颜色", QSize(100,30));
    connect(btnColor, &QPushButton::clicked, this, [=]{
        auto color =  QColorDialog::getColor(Qt::red, this, "选择颜色");
        if(color.isValid()){
            applyColor(color, label);
        }
    });
    mTableWidget->setCellWidget(curRow,3,btnColor);

    QComboBox *classBoxFont = new QComboBox;
    for(int i=10; i<32; i++) {
        classBoxFont->addItem(QString::number(i));
    }
    classBoxFont->setCurrentIndex(3);
    mTableWidget->setCellWidget(curRow, 4, classBoxFont);

    mTableWidget->setItem(curRow, 5, new QTableWidgetItem("双击修改文字内容"));
}


void WatermarkTool::on_btnDelete_clicked()
{
    int curRow = mTableWidget->currentRow();
    mTableWidget->removeRow(curRow);
}


void WatermarkTool::on_btnConfirm_clicked()
{
    m_curWatermark.clear();
    for(int i=0; i<mTableWidget->rowCount(); i++) {
        CustomWatermarkInfo info;
        info.id = QString("水印%1").arg(i);
        info.x = mTableWidget->item(i, 0)->text().toInt();
        info.y = mTableWidget->item(i, 1)->text().toInt();
        QString color = dynamic_cast<QLabel *>(mTableWidget->cellWidget(i,2))->text();
        info.color = QColor(color);

        QString fontSize = dynamic_cast<QComboBox *>(mTableWidget->cellWidget(i,4))->currentText();
        info.fontSize = fontSize.toInt();
        info.value = mTableWidget->item(i, 5)->text();
        m_curWatermark.append(info);
        qDebug() << info.toString();
    }

    emit sigWatermark(m_curWatermark);
}



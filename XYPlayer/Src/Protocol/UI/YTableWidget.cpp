#include "YTableWidget.h"
#include <QHeaderView>
#include <QDebug>

YTableWidget::YTableWidget(QWidget *parent)
    : QTableWidget(parent)
{
    initUI();
}

void YTableWidget::initUI()
{
    int row{1},col{2};
    row = mData.size() >0 ? mData.size() : row;
    setRowCount(row);
    setColumnCount(col);

    horizontalHeader()->setFixedHeight(60);
    verticalHeader()->setFixedWidth(40);
    horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    verticalHeader()->setSectionResizeMode(QHeaderView::Stretch);
#if 0
    //如下代码设置横向表格头的间隔线，有四个方向的间隔线,不需要间隔线的可以设置为0px
    horizontalHeader()->setStyleSheet(
    "QHeaderView::section{"
                "border-top:0px solid #E5E5E5;"
                "border-left:0px solid #E5E5E5;"
                "border-right:0.5px solid #E5E5E5;"
                "border-bottom: 0.5px solid #E5E5E5;"
                "background-color:white;"
                "padding:4px;"
            "}"
    );

    //如下代码设置横向表格头的间隔线，有四个方向的间隔线,不需要间隔线的可以设置为0px
    verticalHeader()->setStyleSheet(
    "QHeaderView::section{"
                "border-top:0px solid #E5E5E5;"
                "border-left:0px solid #E5E5E5;"
                "border-right:0.5px solid #E5E5E5;"
                "border-bottom: 0.5px solid #E5E5E5;"
                "background-color:white;"
                "padding:4px;"
            "}"
    );

    //如下代码设置列表左上角第0行第0列的那个格子的边框线
    verticalHeader()->setStyleSheet(
    "QTableCornerButton::section{"
                "border-top:0px solid #E5E5E5;"
                "border-left:0px solid #E5E5E5;"
                "border-right:0.5px solid #E5E5E5;"
                "border-bottom: 0.5px solid #E5E5E5;"
                "background-color:white;"
            "}"
    );
#endif
//    resize(350, 200); //设置表格
    QStringList headers;
    headers << "名称" << "数据";
    setHorizontalHeaderLabels(headers);
}

void YTableWidget::updateTable()
{
    int cnt{0};
    for(auto it = mData.constBegin();it != mData.constEnd();++it){
        auto itemCol1 = new QTableWidgetItem(it.key());
        auto itemCol2 = new QTableWidgetItem(it.value().toString());
        itemCol1->setTextAlignment(Qt::AlignCenter);
        itemCol2->setTextAlignment(Qt::AlignCenter);
        setItem(cnt,0, itemCol1);
        setItem(cnt,1, itemCol2);
        cnt++;
    }
}

void YTableWidget::readTable()
{
     for (int row = 0; row < rowCount(); ++row) {
        QTableWidgetItem* nameItem  = item(row, 0);
        QTableWidgetItem* valueItem = item(row, 1);
        if (nameItem && valueItem) {
            mData[nameItem->text()] = QVariant(valueItem->text());
        }
    }
}


void YTableWidget::setData(QMap<QString, QVariant> &data)
{
    mData = data;
    clear();
    initUI();
    updateTable();
    update();
}

void YTableWidget::getData(QMap<QString, QVariant> &data)
{
    readTable();
    data = mData;
}



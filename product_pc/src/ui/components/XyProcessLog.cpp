#include "XyProcessLog.h"
#include "ui_XyProcessLog.h"
#include <QScrollBar>
#include "csvlogger.h"
#include <QTextStream>

XyProcessLog::XyProcessLog(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::XyProcessLog)
{
    ui->setupUi(this);

    mTableModel = new QStandardItemModel;
    ui->tableProcessLog->setModel(mTableModel);
    ui->tableProcessLog->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    ui->tableProcessLog->setSelectionBehavior(QAbstractItemView::SelectRows);
    ui->tableProcessLog->setEditTriggers(QAbstractItemView::NoEditTriggers);

    ui->textDebugLog->setReadOnly(true);
    mTableModel->clear();

    mFontSize = 20;
}

XyProcessLog::~XyProcessLog()
{
    delete ui;
}

void XyProcessLog::xyAddTableHeader(QStringList header)
{
    mTableHeader = header;
    mTableModel->setColumnCount(header.size());
    mTableModel->setHorizontalHeaderLabels(header);
}

void XyProcessLog::xyClearTable()
{
    while (mTableModel->rowCount() > 0) {
        mTableModel->removeRow(0);
    }
}

void XyProcessLog::xyAddRowData(QStringList data)
{
    QList<QStandardItem*> list;
    foreach (QString d, data) {
        QStandardItem* pItem = new QStandardItem;
        pItem->setText(d);
        pItem->setTextAlignment(Qt::AlignCenter);
        list.push_back(pItem);
    }

    int row = mTableModel->rowCount();
    mTableModel->insertRow(row, list);

//    QScrollBar* bar = ui->tableProcessLog->verticalScrollBar();
//    bar->setSliderPosition(bar->maximum() + 1);
    ui->tableProcessLog->scrollToBottom();
}

void XyProcessLog::xyUpdateItemData(QString data, int col)
{
    int row = mTableModel->rowCount() - 1;
    QStandardItem* pItem = mTableModel->item(row, col);
    if (!pItem) return;

    pItem->setText(data);
    mTableModel->setItem(row, col, pItem);
}

void XyProcessLog::xyUpdateTestStatus(ItemStatus status)
{
    int row = mTableModel->rowCount() - 1;
    int col = mTableModel->columnCount() - 1;
    QStandardItem* pItem = mTableModel->item(row, col);
    if (!pItem) return;

    QString text = "FAIL";

    QColor clrBack(200, 200, 200);
    QColor clrFont(0, 0, 0);
    switch (status) {
    case StatusPass:
         clrBack = QColor(0, 220, 0);
         clrFont = QColor(255, 255,255);
         text = "PASS";
         break;
    case StatusFail:
         clrBack = QColor(220, 0, 0);
         clrFont = QColor(255, 255,255);
         text = "FAIL";
         break;
    case StatusTesting:
         clrBack = QColor(0, 0, 220);
         clrFont = QColor(255, 255,255);
         text = "TEST";
         break;
    }
    pItem->setText(text);
    pItem->setBackground(QBrush(clrBack));
    pItem->setForeground(QBrush(clrFont));
    mTableModel->setItem(row, col, pItem);
}

void XyProcessLog::xyUpdateRowData(QStringList list, int row)
{
    int currentRow = mTableModel->rowCount() - 1;
    if(row > currentRow) {
        xyAddRowData(list);
    } else {
        for(int i=0; i<list.size(); i++) {
             mTableModel->item(row, i)->setText(list[i]);
        }
    }
}


void XyProcessLog::xyAddPassLog(QString log)
{
    QTextCharFormat fmt;
    fmt.setFontFamily("Microsoft YaHei UI");
    fmt.setForeground(QBrush(Qt::green));
    fmt.setFontPointSize(mFontSize+5);
    ui->textDebugLog->mergeCurrentCharFormat(fmt);
    ui->textDebugLog->appendPlainText(log);

    QScrollBar* bar = ui->textDebugLog->verticalScrollBar();
    bar->setSliderPosition(bar->maximum());
}

void XyProcessLog::xyAddFailLog(QString log)
{
    QTextCharFormat fmt;
    fmt.setFontFamily("Microsoft YaHei UI");
    fmt.setForeground(QBrush(Qt::red));
    fmt.setFontPointSize(mFontSize+5);
    ui->textDebugLog->mergeCurrentCharFormat(fmt);
    ui->textDebugLog->appendPlainText(log);

    QScrollBar* bar = ui->textDebugLog->verticalScrollBar();
    bar->setSliderPosition(bar->maximum());
}

void XyProcessLog::xyAddNormalLog(QString log)
{
    QTextCharFormat fmt;
    fmt.setFontFamily("Microsoft YaHei UI");
    fmt.setForeground(QBrush(Qt::black));
    fmt.setFontPointSize(mFontSize);
    ui->textDebugLog->mergeCurrentCharFormat(fmt);
    ui->textDebugLog->appendPlainText(log);

    QScrollBar* bar = ui->textDebugLog->verticalScrollBar();
    bar->setSliderPosition(bar->maximum());
}

void XyProcessLog::xyAddWarningLog(QString log)
{
    QTextCharFormat fmt;
    fmt.setFontFamily("Microsoft YaHei UI");
    fmt.setForeground(QBrush(Qt::blue));
    fmt.setFontPointSize(mFontSize + 5);
    ui->textDebugLog->mergeCurrentCharFormat(fmt);
    ui->textDebugLog->appendPlainText(log);

    QScrollBar* bar = ui->textDebugLog->verticalScrollBar();
    bar->setSliderPosition(bar->maximum());
}

void XyProcessLog::xyClearLog()
{
    ui->textDebugLog->clear();
}

void XyProcessLog::xySaveCsv(QString log)
{
    CsvLogger logger;
    logger.setCsvLogPath(log);
    logger.addCsvTitle(mTableHeader);

    int row = mTableModel->rowCount();
    int col = mTableModel->columnCount();

    for (int foo = 0; foo < row; foo++) {
        QStringList list;
        for (int fjj = 0; fjj < col; fjj++) {
            list.append(mTableModel->item(foo, fjj)->text());
        }
        logger.addCsvLog(list);
    }
}

void XyProcessLog::xySaveTxt(QString log)
{
    QFile file(log);
    if (!file.open(QIODevice::ReadWrite)) {
        return;
    }

    QTextStream stream(&file);
    stream << ui->textDebugLog->document()->toPlainText();
    stream.flush();
    file.close();
}

void XyProcessLog::xySetFontSize(int size)
{
    mFontSize = size;
}

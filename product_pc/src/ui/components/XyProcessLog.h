#ifndef XYPROCESSLOG_H
#define XYPROCESSLOG_H

#include <QWidget>
#include <QStandardItemModel>

namespace Ui {
class XyProcessLog;
}

typedef enum {
    StatusPass = 0,
    StatusFail,
    StatusTesting
}ItemStatus;

class XyProcessLog : public QWidget
{
    Q_OBJECT
public:
    explicit XyProcessLog(QWidget *parent = nullptr);
    ~XyProcessLog();

    // bool xyCreateUI();
    void xyAddTableHeader(QStringList header);
    void xyClearTable();
    // add data at last
    void xyAddRowData(QStringList data);
    // always update last row's data
    void xyUpdateItemData(QString data, int col);
    // always update last row, last column's background
    void xyUpdateTestStatus(ItemStatus status);
    void xyUpdateRowData(QStringList list, int row);

    void xyAddPassLog(QString log);
    void xyAddFailLog(QString log);
    void xyAddNormalLog(QString log);
    void xyAddWarningLog(QString log);
    void xyClearLog();
    void xySaveCsv(QString log);
    void xySaveTxt(QString log);
    void xySetFontSize(int size);

private:
    Ui::XyProcessLog *ui;
    QStringList mTableHeader;
    QStandardItemModel* mTableModel;
    int mFontSize;
};

#endif // XYPROCESSLOG_H

#ifndef XYMESDEBUG_H
#define XYMESDEBUG_H

#include <QDialog>
#include "mescom.h"
#include <vector>

namespace Ui {
class XyMesDebug;
}

class XyMesDebug : public QDialog
{
    Q_OBJECT

public:
    explicit XyMesDebug(QWidget *parent = nullptr);
    ~XyMesDebug();

private slots:
    void on_btnEnterMes_clicked();

    void on_btnOuterMes_clicked();

    void on_btnBackMes_clicked();

    void on_btnGetSN_clicked();

    void on_btnGetBPList_clicked();

    void on_btnGetSoft_clicked();

    void on_btnGetSoftVer_clicked();

    void on_btnUpload_clicked();

    void on_btnGetPrintTemplate_clicked();

    void on_btnAddParagraph_clicked();

    void on_btnGetSNAppends_clicked();

    void on_btnGetSNByAppends_clicked();

private:
    Ui::XyMesDebug *ui;
    MesCom* mMesPtr;

    QString m_stationName;
    QString m_orderId;
    QString m_userName;
    std::vector<std::tuple<std::string, std::string, bool>> m_dataPtr;
};

#endif // XYMESDEBUG_H

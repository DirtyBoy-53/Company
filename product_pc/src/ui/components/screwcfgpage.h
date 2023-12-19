#ifndef SCREWCFGPAGE_H
#define SCREWCFGPAGE_H

#include <QDialog>
#include <QStandardItemModel>

namespace Ui {
class ScrewCfgPage;
}

class ScrewCfgPage : public QDialog
{
    Q_OBJECT

public:
    explicit ScrewCfgPage(QWidget *parent = 0);
    ~ScrewCfgPage();
    void showInit();

signals:
    void signalUpdateCfg();

public slots:
    void slotUpdateState(const int x, const int y);

private slots:
    void clickedListView(QModelIndex model);
    void on_btnAdd_clicked();

    void on_btnDelete_clicked();

    void on_btnSaveCfgInfo_clicked();

    void on_btnCancle_clicked();

    void on_btnDeleteBefore_clicked();

    void on_btnAddBefore_clicked();

    void on_pushButton_clicked();

    void on_btnConfirm_clicked();

    void on_btnInsertScrew_clicked();

    void on_btnInsertTips_clicked();

    void on_btnClearList_clicked();

private:
    void initTableWidget();
    void initTableTips();
    void initCfgTableWidget();
    void initSysTableWidget();
    void initWorkListWidget();

    Ui::ScrewCfgPage *ui;

    QStandardItemModel* m_model;
    QStandardItemModel* m_modelTips;

    int m_x;
    int m_y;
};

#endif // SCREWCFGPAGE_H

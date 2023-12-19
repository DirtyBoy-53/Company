#ifndef LIFECONTROLSETUP_H
#define LIFECONTROLSETUP_H

#include <QWidget>

#include <QStandardItemModel>
#include <QTableWidgetItem>

namespace Ui {
class LifeControlSetup;
}


typedef struct _customProp{
    QString name;
    QString code;
    int totalTimes;
    int usedTimes;
    int addedTimes;
    QString imgPath;
    _customProp(){
        name = "";
        code = "";
        totalTimes = 0;
        usedTimes  = 0;
        addedTimes = 0;
    }
}CustomProperty;

class LifeControlSetup : public QWidget
{
    Q_OBJECT

public:
    explicit LifeControlSetup(QWidget *parent = nullptr);
    ~LifeControlSetup();

    void show(bool isAdmin = true);
    void showEvent(QShowEvent *event);
private:
    void addRowData(CustomProperty cp);
    void removeRowData();
    void saveData();

signals:
    // void sigOutLimitation();
    void sigResetLabelStatus();
public slots:
    // void slotUpdateUsed();

private slots:

    void on_btnPlus_clicked();

    void on_btnMunus_clicked();

    void on_pushButton_clicked();

    void on_itemChanged(QStandardItem *item);

    void on_pushButton_2_clicked();

private:
    QStandardItemModel* m_itemModel;

private:
    Ui::LifeControlSetup *ui;
};

#endif // LIFECONTROLSETUP_H

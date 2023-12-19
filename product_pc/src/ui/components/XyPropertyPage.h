#ifndef XYPROPERTYPAGE_H
#define XYPROPERTYPAGE_H

#include <QDialog>
#include "CPropertyPage.h"

namespace Ui {
class XyPropertyPage;
}

class XyPropertyPage : public QDialog
{
    Q_OBJECT

public:
    explicit XyPropertyPage(QDialog *parent = nullptr);
    ~XyPropertyPage();
    void showModal(QString path, QString mStationName);

private slots:
    void on_pushButton_clicked();

    void on_btnAdminOperate_clicked();

    void on_btnAddItem_clicked();
signals:
    void sigSave();
private:
    QString mConfigPath;
    Ui::XyPropertyPage *ui;
    CPropertyPage * mPropertyPage;
};

#endif // XYPROPERTYPAGE_H

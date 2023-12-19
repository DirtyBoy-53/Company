#ifndef XYSCANDIALOG_H
#define XYSCANDIALOG_H

#include <QDialog>

namespace Ui {
class XyScanDialog;
}

class XyScanDialog : public QDialog
{
    Q_OBJECT

public:
    explicit XyScanDialog(QWidget *parent = nullptr);
    ~XyScanDialog();

    QString data();
    QStringList dataList()
    {
        return mContentList;
    }
    void showModal(QString labelName = "探测器编码", bool passwd = false);

private slots:
    void on_editSn_returnPressed();

    void on_btnOk_clicked();

    void on_btnCancel_clicked();

private:
    Ui::XyScanDialog *ui;

    QString mContentData;
    QStringList mContentList;
};

#endif // XYSCANDIALOG_H

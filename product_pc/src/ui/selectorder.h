#ifndef SELECTORDER_H
#define SELECTORDER_H

#include <QDialog>
#include <interfaceconstdef.h>

namespace Ui {
class SelectOrder;
}

class SelectOrder : public QDialog
{
    Q_OBJECT

public:
    explicit SelectOrder(QWidget *parent = nullptr);
    ~SelectOrder();

private slots:
    void on_pushButton_clicked();

    void on_comboBox_currentIndexChanged(int index);

private:
    Ui::SelectOrder *ui;

    // QWidget interface
protected:
    void showEvent(QShowEvent *event);
private:
    QList<ResultInfo> m_list;
};

#endif // SELECTORDER_H

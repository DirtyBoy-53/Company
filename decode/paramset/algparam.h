#ifndef ALGPARAM_H
#define ALGPARAM_H

#include <QWidget>
#include "yjsonmanager.h"
namespace Ui {
class AlgParam;
}

class AlgParam : public QWidget
{
    Q_OBJECT

public:
    explicit AlgParam(QWidget *parent = nullptr);
    ~AlgParam();
    void readParamFromUI(shape_json::root_s &root);
    void writeParamToUI(const shape_json::root_s &root);
private slots:
    void on_Btn_ReadParam_clicked();

    void on_Btn_WriteParam_clicked();

private:
    Ui::AlgParam *ui;
    shape_json::root_s m_root;
};

#endif // ALGPARAM_H

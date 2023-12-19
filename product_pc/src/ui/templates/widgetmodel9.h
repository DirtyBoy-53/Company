#ifndef WIDGETMODEL9_H
#define WIDGETMODEL9_H

#include <QWidget>
#include "basecomponent.h"

namespace Ui {
class Widgetmodel9;
}

class Widgetmodel9 : public BaseComponent
{
    Q_OBJECT

public:
    explicit Widgetmodel9(QWidget *parent = 0);
    ~Widgetmodel9();

private slots:

    void on_pushButtonPrint_clicked();

private:
    Ui::Widgetmodel9 *ui;
};

#endif // WIDGETMODEL9_H

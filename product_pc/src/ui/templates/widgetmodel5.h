#ifndef WIDGETMODEL5_H
#define WIDGETMODEL5_H

#include "basecomponent.h"

namespace Ui {
class widgetmodel5;
}

class widgetmodel5 : public BaseComponent
{
    Q_OBJECT

public:
    explicit widgetmodel5(QWidget *parent = nullptr);
    ~widgetmodel5();

private:
    Ui::widgetmodel5 *ui;
};

#endif // WIDGETMODEL5_H

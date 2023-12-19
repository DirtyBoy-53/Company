#ifndef WIDGETMODEL6_H
#define WIDGETMODEL6_H

#include "basecomponent.h"

namespace Ui {
class WidgetModel6;
}

class WidgetModel6 : public BaseComponent
{
    Q_OBJECT

public:
    explicit WidgetModel6(QWidget *parent = nullptr);
    ~WidgetModel6();

private:
    Ui::WidgetModel6 *ui;
};

#endif // WIDGETMODEL6_H

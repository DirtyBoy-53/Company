#ifndef WIDGETMODEL3_H
#define WIDGETMODEL3_H

#include <QWidget>
#include <basecomponent.h>

namespace Ui {
class WidgetModel3;
}

class WidgetModel3 :  public BaseComponent
{
    Q_OBJECT

public:
    explicit WidgetModel3(QWidget *parent = nullptr);
    ~WidgetModel3();

private:
    Ui::WidgetModel3 *ui;
};

#endif // WIDGETMODEL3_H

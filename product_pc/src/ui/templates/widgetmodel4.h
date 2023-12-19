#ifndef WIDGETMODEL4_H
#define WIDGETMODEL4_H

#include <QWidget>
#include <basecomponent.h>

namespace Ui {
class WidgetModel4;
}

class WidgetModel4 :  public BaseComponent
{
    Q_OBJECT

public:
    explicit WidgetModel4(QWidget *parent = nullptr);
    ~WidgetModel4();

private:
    Ui::WidgetModel4 *ui;
};

#endif // WIDGETMODEL4_H

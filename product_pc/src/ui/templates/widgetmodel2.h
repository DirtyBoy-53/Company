#ifndef WIDGETMODEL2_H
#define WIDGETMODEL2_H

#include <QWidget>
#include <basecomponent.h>
namespace Ui {
class WidgetModel2;
}

class WidgetModel2 : public BaseComponent
{
    Q_OBJECT

public:
    explicit WidgetModel2(QWidget *parent = nullptr);
    ~WidgetModel2();

private:
    Ui::WidgetModel2 *ui;
};

#endif // WIDGETMODEL2_H

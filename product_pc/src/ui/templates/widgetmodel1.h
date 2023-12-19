#ifndef WIDGETMODEL1_H
#define WIDGETMODEL1_H

#include <QWidget>
#include <basecomponent.h>

namespace Ui {
class WidgetModel1;
}

class WidgetModel1 : public BaseComponent
{
    Q_OBJECT

public:
    explicit WidgetModel1(QWidget *parent = nullptr);
    ~WidgetModel1();

private:
    Ui::WidgetModel1 *ui;
};

#endif // WIDGETMODEL1_H

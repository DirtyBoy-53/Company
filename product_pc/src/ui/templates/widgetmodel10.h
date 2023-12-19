#ifndef WIDGETMODEL10_H
#define WIDGETMODEL10_H

#include <QWidget>
#include <basecomponent.h>

namespace Ui {
class WidgetModel10;
}

class WidgetModel10 : public BaseComponent
{
    Q_OBJECT

public:
    explicit WidgetModel10(QWidget *parent = 0);
    ~WidgetModel10();

private slots:


private:
    Ui::WidgetModel10 *ui;
};

#endif // WIDGETMODEL10_H

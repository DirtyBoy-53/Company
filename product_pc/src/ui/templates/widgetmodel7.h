#ifndef WIDGETMODEL7_H
#define WIDGETMODEL7_H

#include <QWidget>
#include "basecomponent.h"

namespace Ui {
class WidgetModel7;
}

class WidgetModel7 : public BaseComponent
{
    Q_OBJECT

public:
    explicit WidgetModel7(QWidget *parent = nullptr);
    ~WidgetModel7();

public slots:
    void updateImg(QString path);
    void changePage(const int type);

private:
    Ui::WidgetModel7 *ui;
    QString m_imgPath;
};

#endif // WIDGETMODEL7_H

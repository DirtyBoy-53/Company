#ifndef WIDGETMODEL11_H
#define WIDGETMODEL11_H

#include <QWidget>

namespace Ui {
class widgetmodel11;
}

class widgetmodel11 : public QWidget
{
    Q_OBJECT

public:
    explicit widgetmodel11(QWidget *parent = nullptr);
    ~widgetmodel11();

private:
    Ui::widgetmodel11 *ui;
};

#endif // WIDGETMODEL11_H

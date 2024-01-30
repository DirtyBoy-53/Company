#ifndef SYSTEMPARAMWIDGET_H
#define SYSTEMPARAMWIDGET_H

#include <QWidget>

namespace Ui {
class systemparamwidget;
}

class systemparamwidget : public QWidget
{
    Q_OBJECT

public:
    explicit systemparamwidget(QWidget *parent = 0);
    ~systemparamwidget();

private:
    Ui::systemparamwidget *ui;
};

#endif // SYSTEMPARAMWIDGET_H

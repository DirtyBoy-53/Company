#ifndef XYADDPROPERTY_H
#define XYADDPROPERTY_H

#include <QDialog>

namespace Ui {
class XyAddProperty;
}

class XyAddProperty : public QDialog
{
    Q_OBJECT

public:
    explicit XyAddProperty(QWidget *parent = nullptr);
    ~XyAddProperty();

private:
    Ui::XyAddProperty *ui;
};

#endif // XYADDPROPERTY_H

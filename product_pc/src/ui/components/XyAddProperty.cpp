#include "XyAddProperty.h"
#include "ui_XyAddProperty.h"

XyAddProperty::XyAddProperty(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::XyAddProperty)
{
    ui->setupUi(this);
}

XyAddProperty::~XyAddProperty()
{
    delete ui;
}

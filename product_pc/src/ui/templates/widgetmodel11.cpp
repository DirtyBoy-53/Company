#include "widgetmodel11.h"
#include "ui_widgetmodel11.h"

widgetmodel11::widgetmodel11(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::widgetmodel11)
{
    ui->setupUi(this);
}

widgetmodel11::~widgetmodel11()
{
    delete ui;
}

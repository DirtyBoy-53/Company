#include "widgetmodel4.h"
#include "ui_widgetmodel4.h"

WidgetModel4::WidgetModel4(QWidget *parent) :
    BaseComponent(parent),
    ui(new Ui::WidgetModel4)
{
    ui->setupUi(this);

    setProcessLogWidget(ui->widget);
    setResultWidget(ui->label);
}

WidgetModel4::~WidgetModel4()
{
    delete ui;
}

#include "widgetmodel2.h"
#include "ui_widgetmodel2.h"
#include <QDebug>

WidgetModel2::WidgetModel2(QWidget *parent) :
    BaseComponent(parent),
    ui(new Ui::WidgetModel2)
{
    ui->setupUi(this);
}

WidgetModel2::~WidgetModel2()
{
    delete ui;
}

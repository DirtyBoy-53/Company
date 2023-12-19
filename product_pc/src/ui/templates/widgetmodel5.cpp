#include "widgetmodel5.h"
#include "ui_widgetmodel5.h"

widgetmodel5::widgetmodel5(QWidget *parent) :
    BaseComponent(parent),
    ui(new Ui::widgetmodel5)
{
    ui->setupUi(this);

    setProcessLogWidget(ui->log);
    setChartWidget(ui->chart);
    setResultWidget(ui->result);
}

widgetmodel5::~widgetmodel5()
{
    delete ui;
}

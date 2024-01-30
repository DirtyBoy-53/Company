#include "systemparamwidget.h"
#include "ui_systemparamwidget.h"

systemparamwidget::systemparamwidget(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::systemparamwidget)
{
    ui->setupUi(this);
}

systemparamwidget::~systemparamwidget()
{
    delete ui;
}

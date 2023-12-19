#include "widgetmodel9.h"
#include "ui_widgetmodel9.h"
#include <qglobal.h>

#include "configinfo.h"
#include "StaticSignals.h"

#include "mescom.h"

Widgetmodel9::Widgetmodel9(QWidget *parent) :
    BaseComponent(parent),
    ui(new Ui::Widgetmodel9)
{
    ui->setupUi(this);
    QList <QScreen *> list_screen = QGuiApplication::screens();
    this->resize(list_screen.at(0)->geometry().width(),list_screen.at(0)->geometry().height());
    ui->labelTips->setGeometry(0,0,list_screen.at(0)->geometry().width()*9/10,list_screen.at(0)->geometry().height()/10);
    //ui->widgetVideo->setGeometry(0,list_screen.at(0)->geometry().height()/10,list_screen.at(0)->geometry().width()/2,list_screen.at(0)->geometry().height()*8/10);
    ui->tableWidget->setGeometry(list_screen.at(0)->geometry().width()/2,list_screen.at(0)->geometry().height()/10,list_screen.at(0)->geometry().width()/2,list_screen.at(0)->geometry().height()*7/10);
    ui->pushButtonPrint->setGeometry(list_screen.at(0)->geometry().width()*9/10,0,list_screen.at(0)->geometry().width()/10,list_screen.at(0)->geometry().height()/10);
    //setVideoWidget(ui->widgetVideo);
    setResultWidget(ui->labelTips);
    setProcessLogWidget(ui->tableWidget);
    ui->pushButtonPrint->setVisible(false);

}

Widgetmodel9::~Widgetmodel9()
{
    delete ui;
}

void Widgetmodel9::on_pushButtonPrint_clicked()
{
    if (LOGIN_ADMIN == ConfigInfo::getInstance()->cacheInfo().Permissionmode) {
        StaticSignals::getInstance()->packingNotFullPrint();
    } else {
        QMessageBox::information(nullptr, "提示", "请切换之管理员模式执行打印");
        QString oldUser = MesCom::instance()->getUser();
        QString oldPasswd = MesCom::instance()->getPasswd();
        QString newUser;
        QString newPasswd;
        if (LOGIN_ADMIN == MesCom::instance()->login(newUser, newPasswd)) {
            StaticSignals::getInstance()->packingNotFullPrint();
            MesCom::instance()->login(oldUser, oldPasswd);
            return;
        }


    }
}

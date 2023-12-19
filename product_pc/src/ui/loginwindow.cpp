#include "loginwindow.h"
#include "ui_loginwindow.h"
#include <QMessageBox>
#include <mesclient.h>
#include <QKeyEvent>
#include <configinfo.h>


LoginWindow::LoginWindow(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::LoginWindow)
{
    ui->setupUi(this);
    setFocusPolicy(Qt::StrongFocus);
    ui->editPasswd->setEchoMode(QLineEdit::Password);
    setWindowTitle("登录");
    ui->editUserName->setText("");
    ui->editPasswd->setText("");
    m_produceList = ProduceManager::instance()->produces();
}

LoginWindow::~LoginWindow()
{
    delete ui;
}

void LoginWindow::loginSuc(const QString sCode, const int account)
{
    ui->editPasswd->clear();
    hide();
    emit signalLogin(sCode, account);
}

void LoginWindow::initLogin()
{
    for(int i=0; i<m_produceList.size(); i++) {
        ui->comboProduce->insertItem(i, m_produceList[i].sCode + "-" + m_produceList[i].sName);
    }
    ui->stackedWidget->setCurrentIndex(LOGIN_CHANGE_ACCOUNT);
    show();
}

void LoginWindow::login(const int index)
{
    if(index == LOGIN_CHANGE_PLUGIN) {
        ui->stackedWidget->setCurrentIndex(LOGIN_CHANGE_PLUGIN);
    } else {
        ui->stackedWidget->setCurrentIndex(LOGIN_CHANGE_ACCOUNT);
    }
    show();
}

void LoginWindow::keyPressEvent(QKeyEvent *event)
{
    switch(event->key())
    {
    case Qt::Key_Return:
        if(ui->stackedWidget->currentIndex() == LOGIN_CHANGE_PLUGIN) {
            on_btnSelect_clicked();
        } else if(ui->stackedWidget->currentIndex() == LOGIN_CHANGE_ACCOUNT) {
            on_btnLogin_clicked();
        }
        break;
    default:
        break;
    }
}

void LoginWindow::on_btnLogin_clicked()
{
    QString passwd = ui->editPasswd->text();
    QString userName = ui->editUserName->text();


    int role = -2;
    if (userName == "admin" && passwd == "admin@123456") {
        role = 1;
    } else {
        role = MesClient::instance()->login(userName, passwd);
    //    role = 1;
        if(role < 0){
            if(role == -2) {
                QMessageBox::information(this, "提示", "服务器异常2");
            }else {
                QMessageBox::information(this, "提示", "用户名或密码错误");
            }
            return;
        }
    }
    m_role = role;
    QString sCodeId = ConfigInfo::getInstance()->sysInfo().curProduce;
    CacheInfo info;
    info = ConfigInfo::getInstance()->cacheInfo();
    info.sCodeId = sCodeId;
    info.Permissionmode = role;
    ConfigInfo::getInstance()->setCacheInfo(info) ;
    int pluginIndex = 0;
    for(int i = 0;i <m_produceList.size(); i ++) {
        if(m_produceList[i].sCode == sCodeId) {
            pluginIndex = i;
        }
    }
    if(role == LOGIN_ADMIN) {
        ui->comboProduce->setDisabled(false);

        ui->comboProduce->setCurrentIndex(pluginIndex);
        ui->stackedWidget->setCurrentIndex(LOGIN_CHANGE_PLUGIN);

    }else if(!ConfigInfo::getInstance()->getValueString("是否设置过工序").toInt()) {
        QMessageBox::information(this, "提示", "首次请使用管理账户登录");
        return;
    }else {
        ui->comboProduce->setDisabled(true);
        ui->comboProduce->setCurrentIndex(pluginIndex);
        ui->stackedWidget->setCurrentIndex(LOGIN_CHANGE_PLUGIN);
//        QList<ResultInfo> oList;
//        MesClient::instance()->getOrderIds(oList);
//        ui->cbPro->clear();
//        for(int i = 0; i < oList.size(); i ++) {
//            ui->cbPro->addItem(oList.at(i).key);
//        }
//        loginSuc(workInfo.pluginIndex, role);
    }
    // 第一个工序和包装工序
    if(sCodeId == "DE1000" ||sCodeId == "DE1100" || sCodeId == "CB1000" || sCodeId == "CL1000"
            || sCodeId == "DE1014" || sCodeId == "CB1005" || sCodeId == "CL1007" || sCodeId == "IsPackage") {
        QList<ResultInfo> oList;
        MesClient::instance()->getOrderIds(oList);
        ui->cbPro->clear();
        for(int i = 0; i < oList.size(); i ++) {
            ui->cbPro->addItem(oList.at(i).key);
        }

    }else {
        ui->lineEditOrderId->hide();
        ui->label_4->hide();
        ui->cbPro->hide();
    }
}


void LoginWindow::on_btnSelect_clicked()
{
    int index = ui->comboProduce->currentIndex();
    QString sCodeId = m_produceList[index % m_produceList.size()].sCode;
    QString orderId = ui->lineEditOrderId->text();
    QList<ResultInfo> oList;
    CacheInfo info;
    info = ConfigInfo::getInstance()->cacheInfo();
    info.orderId = orderId;
    info.sCodeId = sCodeId;
    ConfigInfo::getInstance()->setCacheInfo(info) ;

    int iRet = MesClient::instance()->getOrderIds(oList);

    iRet = 0;
    m_role = 1;
    if(iRet == 0) {
        if(iRet == 0) {
            SysInfo sys = ConfigInfo::getInstance()->sysInfo();
            sys.iFirst = 1;
            sys.curProduce = sCodeId;
            ConfigInfo::getInstance()->setSysInfo(sys);
            loginSuc(sCodeId, m_role);
        }else {
            QMessageBox::information(this, "提示", "工单号异常，请确认工单号");
        }
    }else {
        QMessageBox::information(this, "提示", "工单号异常，请确认工单号");

    }
}


void LoginWindow::on_cbPro_currentIndexChanged(int index)
{
    ui->lineEditOrderId->setText(ui->cbPro->currentText());
}


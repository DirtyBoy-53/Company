#include "csm_login.h"
#include "ui_csm_login.h"
#include "qmessagebox.h"
#include <QDebug>

csm_login::csm_login(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::csm_login)
{
    ui->setupUi(this);
    this->setWindowTitle("Login");
    this->setFixedSize(350, 240);
    flag=false;


}



csm_login::~csm_login()
{
    delete ui;
}

void csm_login::on_Loginin_clicked()
{
    QString passwd = ui->key->text();
    QString userName = ui->name->text();
    m_role = MesClient::instance()->login(userName, passwd);
    qDebug() << "role " << m_role;
    if (m_role == 1) {// role 0 1 2
        close();
    } else if (m_role == 0) {
        close();
    } else if (m_role == 2) {
        close();
    } else {
        if(m_role == -2) {
            QMessageBox::information(this, "提示", "服务器异常1");
        }else {
            QMessageBox::information(this, "提示", "密码错误");
        }
        flag=false;
        return;
    }

}

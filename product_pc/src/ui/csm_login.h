#ifndef CSM_LOGIN_H
#define CSM_LOGIN_H

#include <QDialog>
#include <mesclient.h>

namespace Ui {
class csm_login;
}

class csm_login : public QDialog
{
    Q_OBJECT

public:
    explicit csm_login(QWidget *parent = 0);
    ~csm_login();
     bool flag=false;
     int m_role = -1;

private slots:
     void on_Loginin_clicked();

private:
    Ui::csm_login *ui;
};

#endif // CSM_LOGIN_H

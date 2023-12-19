#ifndef LOGINWINDOW_H
#define LOGINWINDOW_H

#include <QWidget>
#include <producemanager.h>
#include <configinfo.h>


enum LoginMode {
    LOGIN_CHANGE_PLUGIN = 0,
    LOGIN_CHANGE_ACCOUNT,
};

namespace Ui {
class LoginWindow;
}

class LoginWindow : public QWidget
{
    Q_OBJECT

public:
    explicit LoginWindow(QWidget *parent = 0);
    ~LoginWindow();
    void initLogin();
    void login(const int index = 0);

signals:
    void signalLogin(const QString sCode, const int account);

protected:
    void keyPressEvent(QKeyEvent *event);

private slots:
    void on_btnLogin_clicked();

    void on_btnSelect_clicked();

    void on_cbPro_currentIndexChanged(int index);

private:
    void loginSuc(const QString sCode, const int account);

    Ui::LoginWindow *ui;
    QVector<ProduceInfo> m_produceList;
    int m_role = 0;

};

#endif // LOGINWINDOW_H

#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include "loginwindow.h"
#include <QStandardItemModel>
#include <QTimer>
#include "interfaceconstdef.h"
#include <basecomponent.h>
#include <baseproduce.h>
#include <XyPropertyPage.h>
#include <atomic>
#include "screwcfgpage.h"
#include <selectorder.h>
#include "comguidecfg.h"
#include "LifeControl.h"
#include "XyLogger.h"

#define MAXDUT 4

enum LabelSytle {
    LABEL_STOP = 0,
    LABEL_WORKING,
    LABEL_FAIL,
    LABEL_PASS
};

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

    void login();
    void initWindow(const QString sCode, const int account);

public slots:
    void slotConnectBtnClicked();
    void slotStartWorkBtnClicked();
    void slotConnectState(int status);
    void slotConfigBtnClicked();
    void slotUpdateStatus();
    void slotSelectOrder();
    // 显示节拍时间
    void slotShowStepCycle(double stepCycle);

protected slots:
    void closeEvent(QCloseEvent *event);
    void slotSaveCfg();

    void slotClearLT();
    void slotSetMaxLT();

signals:
    bool signalResetLifeTime(int total, bool firstStart = false);

private slots:
    void on_actionMES_triggered();

    void on_mangerlogin_clicked();

    void on_actionGuide_triggered();

    void on_actionScrew_triggered();

    void on_actionAlgorithmDebug_triggered();

    void on_actionUserGuide_triggered();

    void on_actionAuthoritySwitch_triggered();

    void on_actionPackagePrint_triggered();

    static void outputOperationRecord(QString log);
private:
    void changeAccountLogin(const int account);

    Ui::MainWindow *ui;
    LoginWindow* m_login;

    QString m_sType;
    QTimer *m_timer;
    BaseComponent* m_widget;
    BaseProduce * m_curProcess;
    XyPropertyPage * m_cfgPage;
    ScrewCfgPage* m_screwCfgPage;
    std::atomic_int m_status;

    int m_uiType;

    SelectOrder * m_selectOrder;
    ComGuideCfg* m_guideCfgWidget;

    static XyLogger * m_logger;
};

#endif // MAINWINDOW_H

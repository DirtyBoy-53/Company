#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QDebug>

#include <mesclient.h>
#include <configinfo.h>
#include <producemanager.h>
#include "widgetmodel1.h"
#include "widgetmodel2.h"
#include "widgetmodel3.h"
#include "widgetmodel4.h"
#include "widgetmodel5.h"
#include "widgetmodel6.h"
#include "widgetmodel7.h"
#include "widgetmodel8.h"
#include "widgetmodel9.h"
#include "widgetmodel10.h"
#include <baseproduce.h>
#include <inputmessagebox.h>
#include <XyPropertyPage.h>
#include <XyMesDebug.h>
#include "csm_login.h"
#include <project_version.h>
#include <QDateTime>
#include <QTime>
#include "configinfoelec.h"
#include "XyAlgorithmDebug.h"
#include "StaticSignals.h"

XyLogger * MainWindow::m_logger = nullptr;

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    setWindowIcon(QIcon(":/logo1.ico"));

    m_login = new LoginWindow();
    m_cfgPage = new XyPropertyPage();
    m_screwCfgPage = new ScrewCfgPage(this);
    m_screwCfgPage->hide();
    m_selectOrder = new SelectOrder(this);
    m_selectOrder->hide();
    ui->stackedWidget->setCurrentIndex(0);
    m_guideCfgWidget = new ComGuideCfg(this);

    connect(m_login, &LoginWindow::signalLogin, this, &MainWindow::initWindow);
    connect(m_cfgPage, &XyPropertyPage::sigSave, this, &MainWindow::slotSaveCfg);
    connect(ui->actionEnvCheck, &QAction::triggered, this, &MainWindow::slotConnectBtnClicked);
    connect(ui->actionStartTest, &QAction::triggered, this, &MainWindow::slotStartWorkBtnClicked);
    connect(ui->actionStopTest, &QAction::triggered, this, &MainWindow::slotStartWorkBtnClicked);
    connect(ui->actionCfg, SIGNAL(triggered()), this, SLOT(slotConfigBtnClicked()));
    connect(ui->actionSetOrder, SIGNAL(triggered()), this, SLOT(slotSelectOrder()));
    connect(ui->actionExitSys, &QAction::triggered, this, &MainWindow::close);

    connect(ui->actionClearLifeTime, &QAction::triggered, this, &MainWindow::slotClearLT);
    connect(ui->actionSetMaxLifeTime, &QAction::triggered, this, &MainWindow::slotSetMaxLT);

    //connect(this, &MainWindow::signalResetLifeTime, ui->widgetLifeTime, &LifeControl::slotResetLifeTime);

    QTimer *timer = new QTimer(this);

    connect(timer, &QTimer::timeout, this, &MainWindow::slotUpdateStatus);
    timer->start(100);

    // 禁用
    ui->btnStartWork->setVisible(false);
    ui->btnConnect->setVisible(false);
    ui->groupBox_2->setVisible(false);
//    ui->btnStartWork->setEnabled(false);
//    ui->btnConnect->setEnabled(false);

    ui->label_status->setStyleSheet("font:bold 20pt \"宋体\";background-color:rgba(255,255,255,0);color:rgba(0,0,220,255);text-align:center;qproperty-alignment: AlignCenter;");
    setStyleSheet("background-color:rgba(250,250,250,250)");

    ui->actionStartTest->setEnabled(false);
    ui->actionStopTest->setEnabled(false);
    ui->actionEnvCheck->setEnabled(true);


    ui->stackedWidget->setCurrentIndex(0);
    ui->labelLogo->setFixedSize(90, 70);
    ui->labelLogo->setPixmap(QPixmap::fromImage(QImage(":/logo1.png")));
    ui->labelLogo->setScaledContents(true);
    //ui->btnDisConnect->setEnabled(false);
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::closeEvent(QCloseEvent *event)
{
    QMessageBox::StandardButton btn = QMessageBox::information(this, "确定？", "是否确定退出程序？", QMessageBox::Ok | QMessageBox::Cancel, QMessageBox::Cancel);
    if (btn == QMessageBox::Cancel) {
        event->ignore();
        return;
    }
    m_login->close();
    m_cfgPage->close();
    m_selectOrder->close();
}

void MainWindow::slotSaveCfg()
{
    outputOperationRecord("点击参数配置-保存按钮");
    ConfigInfo::getInstance()->ReadAllConfig();
    QString produceName = ConfigInfo::getInstance()->getValueString("项目选择");
    ConfigInfoElecLock::getInstance()->changeProduce(produceName);   
    m_guideCfgWidget->changeProduce(produceName);

    // 保存配置档后需要重新做初始化
    ui->btnConnect->setEnabled(true);
    ui->btnStartWork->setEnabled(false);
}

void MainWindow::slotClearLT()
{
    int nLifeTime = 0;
    ConfigInfo::getInstance()->getValueInt32("GLOBAL", "MaxLifeTime", nLifeTime);
    emit signalResetLifeTime(nLifeTime, false);
}

void MainWindow::slotSetMaxLT()
{
    int nLifeTime = 0;
    ConfigInfo::getInstance()->getValueInt32("GLOBAL", "MaxLifeTime", nLifeTime);
    emit signalResetLifeTime(nLifeTime, true);
}

void MainWindow::changeAccountLogin(const int account)
{
    if(account == LOGIN_NORMAL) {
        ui->actionChangePlugin->setEnabled(false);
    } else if(account == LOGIN_ADMIN) {
        ui->actionChangePlugin->setEnabled(true);
    }
}

void MainWindow::login()
{
    m_login->initLogin();
}

void MainWindow::initWindow(const QString sCode, const int account)
{
    if (sCode == "DE1000" || sCode == "DE1100" || sCode == "CB1005"
            || sCode == "CB1000" || sCode == "CL1000"
            || sCode == "CL1007" || sCode == "DE1014" || sCode == "IsPackage") {
        ui->actionSetOrder->setVisible(true);
    } else {
        ui->actionSetOrder->setVisible(false);
    }
    if (sCode == "DE1014" || sCode == "CL1007" || sCode == "CB1005" || sCode == "IsPackage") {
        ui->actionPackagePrint->setVisible(true);
    } else {
        ui->actionPackagePrint->setVisible(false);
    }

    changeAccountLogin(account);
    qInfo("sCode %s", sCode.toStdString().data());
    int uiType = ProduceManager::instance()->getUiTemplate(sCode);
    qInfo("sCode %s uiType %d", sCode.toStdString().data(),uiType);
    QString buildTm=QString::asprintf("%s %s",__DATE__,__TIME__);
    setWindowTitle(ProduceManager::instance()->getTitle(sCode) + " "+ PROJECT_VERSION +" build "+  buildTm);
    m_curProcess = ProduceManager::instance()->getCurProduce();
    ConfigInfo::getInstance()->ReadAllConfig();
    if (uiType == 1) {
        m_widget = new WidgetModel1;
    } else if(uiType == 2) {
        m_widget = new WidgetModel2;
    } else if(uiType == 3) {
        m_widget = new WidgetModel3;
    } else if(uiType == 4) {
        m_widget = new WidgetModel4;
    } else if(uiType == 5) {
        m_widget = new widgetmodel5;
    } else if(uiType == 6) {
        m_widget = new WidgetModel6;
    } else if(uiType == 7) {
        m_widget = new WidgetModel7;
    } else if(uiType == 8) {
        m_widget = new WidgetModel8;
    } else if(uiType == 9) {
        m_widget = new Widgetmodel9;
    } else if(uiType == 10) {
        m_widget = new WidgetModel10;
    } else {
        return;
    }
    m_uiType = uiType;

    if(uiType == 7) {
        ui->actionGuide->setVisible(true);
    } else {
        ui->actionGuide->setVisible(false);
    }

    if(uiType == 8) {
        ui->actionScrew->setVisible(true);
    } else {
        ui->actionScrew->setVisible(false);
    }

    connect(m_curProcess,&BaseProduce::updateStatus,this,&MainWindow::slotConnectState);
    connect(m_curProcess,&BaseProduce::signalStepCycle,this,&MainWindow::slotShowStepCycle);

    connect(m_curProcess,&BaseProduce::showMsgBox,m_widget,&BaseComponent::showMsgBox);
    connect(m_curProcess,&BaseProduce::hideMsgBox,m_widget,&BaseComponent::hideMsgBox);

    connect(m_widget,&BaseComponent::closeBox,m_curProcess,&BaseProduce::slotBoxClosed);
    connect(m_widget,&BaseComponent::signalClickedFinish,m_curProcess,&BaseProduce::slotClickedFinish);

    // video
    connect(m_curProcess, &BaseProduce::videoUpdate,m_widget,&BaseComponent::slotVideoUpdate);
    connect(m_curProcess, &BaseProduce::waterPrintAddin, m_widget, &BaseComponent::slotWaterPrintAddin);
    connect(m_curProcess, &BaseProduce::waterPrintVisible, m_widget, &BaseComponent::slotWaterPrintVisible);
    connect(m_curProcess, &BaseProduce::startRecord, m_widget, &BaseComponent::slotStartRecord);
    connect(m_curProcess, &BaseProduce::stopRecord, m_widget, &BaseComponent::slotStopRecord);

    // chart
    connect(m_curProcess, &BaseProduce::chartClearGraph, m_widget, &BaseComponent::slotChartClearGraph);
    connect(m_curProcess, &BaseProduce::chartClearData, m_widget, &BaseComponent::slotChartClearData);
    connect(m_curProcess, &BaseProduce::chartAddGraph, m_widget, &BaseComponent::slotChartAddGraph);
    connect(m_curProcess, &BaseProduce::chartAddLine, m_widget, &BaseComponent::slotChartAddLine);
    connect(m_curProcess, &BaseProduce::chartSetXRange, m_widget, &BaseComponent::slotChartXRange);
    connect(m_curProcess, &BaseProduce::chartSetYRange, m_widget, &BaseComponent::slotChartYRange);
    connect(m_curProcess, &BaseProduce::chartAddData, m_widget, &BaseComponent::slotChartAddData);
    connect(m_curProcess, &BaseProduce::chartSetLegendAlign, m_widget, &BaseComponent::slotChartLegendAlign);

    // table
    connect(m_curProcess, &BaseProduce::tableAddHeader, m_widget, &BaseComponent::slotTableAddHeader);
    connect(m_curProcess, &BaseProduce::tableAddRowData, m_widget, &BaseComponent::slotTableAddRow);
    connect(m_curProcess, &BaseProduce::tableUpdateData, m_widget, &BaseComponent::slotTableUpdateItem);
    connect(m_curProcess, &BaseProduce::tableUpdateRowData, m_widget, &BaseComponent::slotTableUpdateRow);

    connect(m_curProcess, &BaseProduce::tableUpdateTestStatus, m_widget, &BaseComponent::slotTableUpdateStatus);
    connect(m_curProcess, &BaseProduce::tableClearData, m_widget, &BaseComponent::slotTableClear);
    connect(m_curProcess, &BaseProduce::tableSaveCsv, m_widget, &BaseComponent::slotTableSaveCsv);

    // log
    connect(m_curProcess, &BaseProduce::logClearData, m_widget, &BaseComponent::slotLogClear);
    connect(m_curProcess, &BaseProduce::logAddPassLog, m_widget, &BaseComponent::slotLogAddPass);
    connect(m_curProcess, &BaseProduce::logAddFailLog, m_widget, &BaseComponent::slotLogAddFail);
    connect(m_curProcess, &BaseProduce::logAddNormalLog, m_widget, &BaseComponent::slotLogAddNormal);
    connect(m_curProcess, &BaseProduce::logAddWarningLog, m_widget, &BaseComponent::slotLogAddWarning);
    connect(m_curProcess, &BaseProduce::logSaveTxt, m_widget, &BaseComponent::slotLogSaveTxt);

    // label
    connect(m_curProcess, &BaseProduce::resultShowDefault, m_widget, &BaseComponent::slotResultShowDefault);
    connect(m_curProcess, &BaseProduce::resultShowPass, m_widget, &BaseComponent::slotResultShowPass);
    connect(m_curProcess, &BaseProduce::resultShowFail, m_widget, &BaseComponent::slotResultShowFail);
    connect(m_curProcess, &BaseProduce::resultShowProcess, m_widget, &BaseComponent::slotShowProcess);

    // 包装用
    connect(m_widget, &BaseComponent::sigPrint, m_curProcess, &BaseProduce::slotprintSN);

    // 耗材寿命管控
    //connect(this, &MainWindow::signalResetLifeTime, ui->widgetLifeTime, &LifeControl::slotResetLifeTime);
    connect(m_curProcess, &BaseProduce::updateLifeTime, ui->widgetLifeTime, &LifeControl::updateStatusLife);
    connect(ui->widgetLifeTime, &LifeControl::sigOutLimitation, m_curProcess, &BaseProduce::slotLifeTimeOutLimit);
    {
        int nLifeTime = 0;
        bool bLifeControl = false;
        ConfigInfo::getInstance()->getValueBoolean("GLOBAL", "耗材寿命管控", bLifeControl);
        if (bLifeControl) {
            ui->widgetLifeTime->setVisible(true);
        } else {
            ui->widgetLifeTime->setVisible(false);
        }

        int authority = ConfigInfo::getInstance()->cacheInfo().Permissionmode;
        if (bLifeControl && authority == LOGIN_ADMIN) {
            ui->actionClearLifeTime->setEnabled(true);
            ui->actionSetMaxLifeTime->setEnabled(true);
        } else {
            ui->actionClearLifeTime->setEnabled(false);
            ui->actionSetMaxLifeTime->setEnabled(false);
        }
        ConfigInfo::getInstance()->getValueInt32("GLOBAL", "MaxLifeTime", nLifeTime);
        emit signalResetLifeTime(nLifeTime, true); // 第一次启动，从配置文件读取耗材使用次数
    }

    ui->widget->layout()->addWidget(m_widget);
    showMaximized();    
	m_status = 1;
}

void MainWindow::slotConnectBtnClicked()
{
    m_curProcess->slotConnectBtnClicked();
}

void MainWindow::slotStartWorkBtnClicked()
{
    if(m_status == 1) {
        m_status = 0;
        //ui->btnStartWork->setText("停止工作");
        ui->actionStartTest->setEnabled(false);
        ui->actionStopTest->setEnabled(true);
        m_curProcess->slotStartWorkBtnClicked();
    } else {
        m_status = 1;
        ui->actionStartTest->setEnabled(true);
        ui->actionStopTest->setEnabled(false);
        //ui->btnStartWork->setText("开始工作");
        m_curProcess->slotStopWorkBtnClicked();
    }
}

void MainWindow::slotConnectState(int status)
{
    if (status == 1) {
        ui->actionStartTest->setEnabled(true);
        ui->actionStopTest->setEnabled(true);
        // ui->btnStartWork->setEnabled(true);
        ui->actionEnvCheck->setEnabled(false);
    } else if (status == 0) {
        ui->actionStartTest->setEnabled(false);
        ui->actionStopTest->setEnabled(false);
        // ui->btnStartWork->setEnabled(true);
        ui->actionEnvCheck->setEnabled(true);
    } else if (status == 2) {
        ui->actionStartTest->setEnabled(true);
        // ui->btnStartWork->setEnabled(true);
        m_status = 1;
        ui->btnStartWork->setText("开始工作");
    }
}



void MainWindow::slotConfigBtnClicked()
{
    outputOperationRecord("点击参数配置按钮");
    if (ConfigInfo::getInstance()->isOperator()) {
        m_cfgPage->setEnabled(false);
    } else {
        m_cfgPage->setEnabled(true);
    }
    m_cfgPage->showModal(ConfigInfo::getInstance()->configPath(),
                         ConfigInfo::getInstance()->cacheInfo().sCodeId);
}

void MainWindow::slotUpdateStatus()
{
    QString sStatusInfo = "用户 " + MesClient::instance()->sUser();

    QString projName = "";
    ConfigInfo::getInstance()->getValueString("GLOBAL", "项目选择", projName);
    CacheInfo info = ConfigInfo::getInstance()->cacheInfo();
    sStatusInfo.append(" 项目 " + projName);
    sStatusInfo.append(" 工单号 " + info.orderId);
    sStatusInfo.append(" 编号 " + info.sSn);
    ui->label_status->setText(sStatusInfo);

    SysInfo sysInfo;
    sysInfo = ConfigInfo::getInstance()->sysInfo();
    if(sysInfo.mode == 0) {
        ui->labelWorkMode->setStyleSheet("color:rgb(0,150,0);font-size:30px;font-weight:Bold;qproperty-alignment: AlignCenter;");
        ui->labelWorkMode->setText("生产模式");
    } else if(sysInfo.mode == 1) {
        ui->labelWorkMode->setStyleSheet("color:rgb(250,0,0);font-size:30px;font-weight:Bold;qproperty-alignment: AlignCenter;");
        ui->labelWorkMode->setText("点检模式");
    } else if(sysInfo.mode == 2) {
        ui->labelWorkMode->setStyleSheet("color:rgb(150,150,0);font-size:30px;font-weight:Bold;qproperty-alignment: AlignCenter;");
        ui->labelWorkMode->setText("调试模式");
    }

    if(info.Permissionmode == 1)
     {
         ui->mangerlogin->setEnabled(false);
     }
     else
     {
         ui->mangerlogin->setEnabled(true);
     }
}

void MainWindow::slotSelectOrder()
{
    if(m_selectOrder->isHidden()) {
        m_selectOrder->show();
    }else {
        m_selectOrder->hide();
    }
}

void MainWindow::slotShowStepCycle(double stepCycle)
{
    ui->labelStepCycle->setText(QString::number(stepCycle, 'f', 2) + "s");
}

void MainWindow::on_actionMES_triggered()
{
    XyMesDebug* debug = new XyMesDebug;
    debug->show();
}


void MainWindow::on_mangerlogin_clicked()
{
//    csm_login* csm_log = new csm_login;
//    csm_log->show();
//    csm_log->exec();  //等待用户相应csm_login
//    if(csm_log->flag)
//    {
//        CacheInfo info;
//        info = ConfigInfo::getInstance()->cacheInfo();
//        info.Permissionmode = 1;
//        ConfigInfo::getInstance()->setCacheInfo(info);
//        ui->mangerlogin->setEnabled(false);

//    }
}

void MainWindow::on_actionGuide_triggered()
{
    m_guideCfgWidget->show();
}


void MainWindow::on_actionScrew_triggered()
{
    outputOperationRecord("点击螺丝批参数配置按钮");
    if (ConfigInfo::getInstance()->isOperator()) {
        m_screwCfgPage->setEnabled(false);
    } else {
        m_screwCfgPage->setEnabled(true);
    }
    QString product = ConfigInfo::getInstance()->getValueString("项目选择");
    ConfigInfoElecLock::getInstance()->changeProduce(product);
    m_screwCfgPage->showInit();
}


void MainWindow::on_actionAlgorithmDebug_triggered()
{
    // 此处会存在内存问题，考虑开关次数少，没有处理
    XyAlgorithmDebug* debug = new XyAlgorithmDebug;
    debug->show();
}


void MainWindow::on_actionUserGuide_triggered()
{
    QString stationName = ConfigInfo::getInstance()->cacheInfo().sCodeId;
    // QString pathGuideFile = "D:/NC9001A 板卡改制说明.pdf";
    QString pathGuideFile = QCoreApplication::applicationDirPath() + "/help/" + stationName + ".pdf";

    QDesktopServices::openUrl(QUrl::fromLocalFile(pathGuideFile));
}


void MainWindow::on_actionAuthoritySwitch_triggered()
{
    outputOperationRecord("点击权限配置按钮");
    csm_login* csm_log = new csm_login;
    csm_log->show();
    csm_log->exec();  //等待用户相应csm_login
    if(csm_log->m_role >= 0)
    {
        CacheInfo info;
        info = ConfigInfo::getInstance()->cacheInfo();
        info.Permissionmode = csm_log->m_role;
        ConfigInfo::getInstance()->setCacheInfo(info);
        qDebug() << "用户身份切换OK";
    }
}


void MainWindow::on_actionPackagePrint_triggered()
{
    if (LOGIN_ADMIN == ConfigInfo::getInstance()->cacheInfo().Permissionmode) {
        qDebug() << "here";
        StaticSignals::getInstance()->packingNotFullPrint();
    } else {
        qDebug() << "here 2";
        QString oldUser = MesClient::instance()->sUser();
        QString oldPasswd = MesClient::instance()->sPass();
        qDebug() << "old " << oldUser << " " << oldPasswd;
        on_actionAuthoritySwitch_triggered();
        if (LOGIN_ADMIN == ConfigInfo::getInstance()->cacheInfo().Permissionmode) {
            StaticSignals::getInstance()->packingNotFullPrint();
            int role = MesClient::instance()->login(oldUser, oldPasswd);
            if (0 != role) {
                QMessageBox::information(this, "tips", "操作员用户切换失败，请手动切换");
                return;
            }
            CacheInfo info;
            info = ConfigInfo::getInstance()->cacheInfo();
            info.Permissionmode = role;
            ConfigInfo::getInstance()->setCacheInfo(info);
        } else {
            QMessageBox::information(this, "tips", "未满箱打印，请使用管理员权限登录");
        }
    }
}

#include "XyLogger.h"
void MainWindow::outputOperationRecord(QString log)
{
    QString user = MesClient::instance()->sUser();
    QString path = QCoreApplication::applicationDirPath() + "/ProcessLog.log";
    if (!m_logger)
        m_logger = new XyLogger("OperationLog", path.toStdString());

    QString logx = log + " [用户：" + user + "]";
    m_logger->infor(logx.toStdString());
}


#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QLabel>
#include <QDebug>
MainWindow::MainWindow(QWidget *parent):
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    setWindowIcon(QIcon(":/image/truthsystem.png"));
    Log::initLog();
    init();
    
    showMaximized();
}
void QComboBox::wheelEvent(QWheelEvent *e){}
MainWindow::~MainWindow()
{
    if(m_cameraparamwidget){
        delete m_cameraparamwidget;
        m_cameraparamwidget = nullptr;
    }
    if(m_systemSetWidget){
        delete m_systemSetWidget;
        m_systemSetWidget = nullptr;
    }
    if(m_eventWidget){
        delete m_eventWidget;
        m_eventWidget = nullptr;
    }
    if(m_transwidget){
        delete m_transwidget;
        m_transwidget = nullptr;
    }

    if(m_cameradiswidget){
        delete m_cameradiswidget;
        m_cameradiswidget = nullptr;
    }    
    if(m_playBackUi){
        delete m_playBackUi;
        m_playBackUi = nullptr;
    }    
    if(m_plotViewer){
        delete m_plotViewer;
        m_plotViewer = nullptr;
    }    
    if(m_lidarViewerWidget){
        delete m_lidarViewerWidget;
        m_lidarViewerWidget = nullptr;
    }
    if(m_sensorFusion){
        delete m_sensorFusion;
        m_sensorFusion = nullptr;
    }
    if(m_matchFusionWidget){
        delete m_matchFusionWidget;
        m_matchFusionWidget = nullptr;
    }
}

/// @brief 初始化
void MainWindow::init()
{
    //设置状态栏信息

    //将状态栏的所有item边框宽度设置为0
    // statusBar()->setStyleSheet(“QStatusBar::item{border: 0px}”); 
    QLabel *_versionLabel = new QLabel("版本信息:V1.3 20231207    ");
    statusBar()->addPermanentWidget(_versionLabel,0);

    //设置侧边栏
    ui->stack_sidewidget->hide();

    m_cameraparamwidget = new cameraparamwidget();
    m_systemSetWidget = new SystemSetWidget();
    m_eventWidget = new EventWidget();
    m_transwidget = new TransConfig();



    ui->stack_sidewidget->insertWidget(0,m_cameraparamwidget);
    ui->stack_sidewidget->insertWidget(1,m_eventWidget);
    ui->stack_sidewidget->insertWidget(2,m_systemSetWidget);
    


    ui->stack_sidewidget->setCurrentIndex(0);

    // connect(ui->bt_sidebar,&QPushButton::clicked,this,&MainWindow::slot_bt_clicked);
    connect(ui->bt_cameraparam,&QPushButton::clicked,this,&MainWindow::slot_bt_clicked);
    connect(ui->bt_eventSet,&QPushButton::clicked,this,&MainWindow::slot_bt_clicked);
    connect(ui->bt_systemparam,&QPushButton::clicked,this,&MainWindow::slot_bt_clicked);


    //设置主窗口
    m_cameradiswidget = new cameradiswidget();
    m_playBackUi = new PlayBackUi();
    m_plotViewer = new PlotViewer();
    m_lidarViewerWidget = new LidarViewerWidget();
    m_sensorFusion = new SensorFusion();
    m_matchFusionWidget = new MatchFusionWidget();

    ui->stack_mainwidget->insertWidget(0,m_cameradiswidget);
    ui->stack_mainwidget->insertWidget(1,m_lidarViewerWidget);
    ui->stack_mainwidget->insertWidget(2,m_plotViewer);
    ui->stack_mainwidget->insertWidget(3,m_playBackUi);
    ui->stack_mainwidget->insertWidget(4,m_sensorFusion);
    ui->stack_mainwidget->insertWidget(5,m_matchFusionWidget);

    ui->stack_mainwidget->setCurrentIndex(0);
    ui->bt_dev_calibration->hide();

    connect(ui->bt_videodis,&QPushButton::clicked,this,&MainWindow::slot_bt_clicked);
    connect(ui->bt_lidardis,&QPushButton::clicked,this,&MainWindow::slot_bt_clicked);
    connect(ui->bt_radardis,&QPushButton::clicked,this,&MainWindow::slot_bt_clicked);
    connect(ui->bt_dev_replay,&QPushButton::clicked,this,&MainWindow::slot_bt_clicked);
    connect(ui->bt_sensor_fusion,&QPushButton::clicked,this,&MainWindow::slot_bt_clicked);
    connect(ui->bt_match_fusion,&QPushButton::clicked,this,&MainWindow::slot_bt_clicked);

    connect(ui->bt_dev_calibration,&QPushButton::clicked,this,&MainWindow::slot_bt_clicked);
    

    // connect(m_cameraparamwidget,&cameraparamwidget::signal_lidar_view_dir,m_cameradiswidget,&cameradiswidget::slot_lidar_view_dir);
    connect(m_systemSetWidget,&SystemSetWidget::signal_sendSavePath,m_lidarViewerWidget,&LidarViewerWidget::slot_set_lidar_path);
    connect(m_systemSetWidget,&SystemSetWidget::signal_sendSavePath,m_plotViewer,&PlotViewer::slot_set_radar_path);
}



/// @brief 按钮响应函数
void MainWindow::slot_bt_clicked()
{
    QPushButton* btn = (QPushButton*)sender();
    QString btnName = btn->objectName();
    static int _local{0};
    //侧边栏按钮
    if (btnName == "bt_sidebar"){
        // if(ui->stack_sidewidget->isHidden()){
        //     ui->stack_sidewidget->show();
        //     ui->bt_sidebar->setIcon(QIcon(":/image/zuozuo.png"));
        // }else{
        //     ui->stack_sidewidget->hide();
        //     ui->bt_sidebar->setIcon(QIcon(":/image/youyou.png"));
        // }
    }else if(btnName == "bt_cameraparam"){
        ui->stack_sidewidget->setCurrentIndex(0);
        if(ui->stack_sidewidget->isHidden()){
            ui->stack_sidewidget->show();
        }else{
            if(_local == 1)
            ui->stack_sidewidget->hide();
        }
        _local = 1;
    }else if(btnName == "bt_eventSet"){
        ui->stack_sidewidget->setCurrentIndex(1);
        if(ui->stack_sidewidget->isHidden()){
            ui->stack_sidewidget->show();
        }else{
            if(_local == 2)
            ui->stack_sidewidget->hide();
        }
        _local = 2;
    }else if(btnName == "bt_systemparam"){
        ui->stack_sidewidget->setCurrentIndex(2);
        if(ui->stack_sidewidget->isHidden()){
            ui->stack_sidewidget->show();
        }else{
            if(_local == 3)
            ui->stack_sidewidget->hide();
        }
        _local = 3;
    }
    //主窗口按钮
    else if(btnName == "bt_videodis"){
        ui->stack_mainwidget->setCurrentIndex(0);
    }else if(btnName == "bt_lidardis"){
        ui->stack_mainwidget->setCurrentIndex(1);
    }else if(btnName == "bt_radardis"){
        ui->stack_mainwidget->setCurrentIndex(2);
    }else if(btnName == "bt_dev_replay"){
        ui->stack_mainwidget->setCurrentIndex(3);
    }else if(btnName == "bt_sensor_fusion"){
        ui->stack_mainwidget->setCurrentIndex(4);
    }else if(btnName == "bt_match_fusion"){
        ui->stack_mainwidget->setCurrentIndex(5);
    }
}

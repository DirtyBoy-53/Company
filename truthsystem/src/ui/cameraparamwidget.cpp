#include "cameraparamwidget.h"
#include "ui_cameraparamwidget.h"

cameraparamwidget::cameraparamwidget(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::cameraparamwidget)
{
    ui->setupUi(this);
    init();
}

cameraparamwidget::~cameraparamwidget()
{
    if(m_transConfig){
        delete m_transConfig;
        m_transConfig = nullptr;
    }
    delete ui;
}

void cameraparamwidget::init()
{
    ui->bt_view_front->setIcon(QIcon(":/image/front.png"));
    ui->bt_view_right->setIcon(QIcon(":/image/right.png"));
    ui->bt_view_top->setIcon(QIcon(":/image/top.png"));


    connect(ui->bt_paramSet,&QPushButton::clicked,this,&cameraparamwidget::slot_bt_Chick);
    connect(ui->bt_saveSet,&QPushButton::clicked,this,&cameraparamwidget::slot_bt_Chick);
    connect(ui->bt_setConfig,&QPushButton::clicked,this,&cameraparamwidget::slot_bt_Chick);

    connect(ui->bt_view_front,&QPushButton::clicked,this,&cameraparamwidget::slot_bt_Chick);
    connect(ui->bt_view_right,&QPushButton::clicked,this,&cameraparamwidget::slot_bt_Chick);
    connect(ui->bt_view_top,&QPushButton::clicked,this,&cameraparamwidget::slot_bt_Chick);
}

void cameraparamwidget::slot_bt_Chick()
{    
    QPushButton* btn = (QPushButton*)sender();
    QString btnName = btn->objectName();
    //按钮
    if (btnName == "bt_paramSet"){
        if(m_transConfig == nullptr){
            m_transConfig = new TransConfig();
        }
        m_transConfig->show();
    }else if(btnName == "bt_saveSet"){
        if(m_recordConfig == nullptr){
            m_recordConfig = new RecordConfig();
        }
        m_recordConfig->show();
    }else if(btnName == "bt_setConfig"){
        
    }else if(btnName == "bt_view_front"){
        signal_lidar_view_dir(ENUM_VIEW::Enum_front);
    }else if(btnName == "bt_view_right"){
        signal_lidar_view_dir(ENUM_VIEW::Enum_right);
    }else if(btnName == "bt_view_top"){
        signal_lidar_view_dir(ENUM_VIEW::Enum_top);
    }
    
}
#include "plotviewer.h"
#include "ui_plotviewer.h"
#include <QFont>

PlotViewer::PlotViewer(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::PlotViewer)
{
    ui->setupUi(this);
    init();

    //进制该页面上的滚轮事件，防止误触：
    auto qComboBoxGroup= this->findChildren<QComboBox*>();
    for(auto each:qComboBoxGroup)
    {
    each->installEventFilter(this);
    }

    new std::thread(&PlotViewer::doWork,this);
    connect(ui->comboBox_type0,QOverload<const QString&>::of(&QComboBox::currentIndexChanged),[this](const QString& str){
        if(str == "主雷达"){
            m_dev0_is_master_radar = true;
             ui->comboBox_type1->setCurrentIndex(1);
        }else {
            m_dev0_is_master_radar = false;
            ui->comboBox_type1->setCurrentIndex(0);
        }
       
    });
    connect(ui->comboBox_type1,QOverload<const QString&>::of(&QComboBox::currentIndexChanged),[this](const QString& str){
        if(str == "主雷达"){
            m_dev0_is_master_radar = false;
            ui->comboBox_type0->setCurrentIndex(1);
        }else {
            m_dev0_is_master_radar = true;
            ui->comboBox_type0->setCurrentIndex(0);
        }
        
    });
    connect(ui->btn_connect0,&QPushButton::clicked,[this](){
            if(ui->btn_connect0->text() == "连接"){
                radar_init(0); 
                ui->btn_connect0->setText("断开");
            }else{
                ui->btn_connect0->setText("连接");
                if(m_radar_ars408[0]){
                    m_radar_ars408[0]->close();
                    // delete m_radar_ars408[0];
                    // m_radar_ars408[0] = nullptr;
                }
            } 
        });
    connect(ui->btn_connect1,&QPushButton::clicked,[this](){ 
        if(ui->btn_connect1->text() == "连接"){
                radar_init(1); 
                ui->btn_connect1->setText("断开");
            }else{
                ui->btn_connect1->setText("连接");
                if(m_radar_ars408[1]){
                    m_radar_ars408[1]->close();
                    // delete m_radar_ars408[1];
                    // m_radar_ars408[1] = nullptr;
                }
            } 
    });

    connect(ui->btn_save0,&QPushButton::clicked,[this](){
            if(m_radar_ars408[0]){
                if(ui->btn_save0->text() == "保存"){
                    m_radar_ars408[0]->setPath(m_savePath);
                    m_radar_ars408[0]->startSave(); 
                    ui->btn_save0->setText("停止");
                }else{
                    m_radar_ars408[0]->stopSave(); 
                    ui->btn_save0->setText("保存");
                }
            }else{
                QMessageBox::information(this, "Tips", "please connect radar.");
            }
        });

    connect(ui->btn_save1,&QPushButton::clicked,[this](){
        if(m_radar_ars408[1]){
            if(ui->btn_save1->text() == "保存"){
                m_radar_ars408[1]->setPath(m_savePath);
                m_radar_ars408[1]->startSave(); 
                ui->btn_save1->setText("停止");
            }else{
                m_radar_ars408[1]->stopSave(); 
                ui->btn_save1->setText("保存");
            }
        }else{
            QMessageBox::information(this, "Tips", "please connect radar.");
        }
    });

}

PlotViewer::~PlotViewer()
{
    delete ui;
}

void PlotViewer::slot_RadaroneFrameProc(StructRadarDataPtr data)
{
    // m_queue.push(data);
    QVector<double> _x,_y,_id;
    int _devIdx{0};
    _devIdx = getDevIdx(data.get()->_idx,data.get()->_devID);
    for(auto i = 0;i < data.get()->_objNum;i++){
        _x.push_back(data.get()->_RadarInfo[i]._x );
        _y.push_back(data.get()->_RadarInfo[i]._y );
        _id.push_back(data.get()->_RadarInfo[i]._id);
    }
    dis_plot(_devIdx,_id,_x,_y);
    // _x.clear(); _y.clear(); _id.clear();
}

bool PlotViewer::eventFilter(QObject *target, QEvent *event)
{
        //屏蔽鼠标滚轮事件
    if(event->type() == QEvent::Wheel&&(target->inherits("QComboBox")))
    {
        return true;
    }
    return false;
}

void PlotViewer::init()
{
    ui->comboBox_type0->setCurrentIndex(0);
    ui->comboBox_type1->setCurrentIndex(1);
    m_customPlot[0] = ui->widget_plot;
    m_customPlot[1] = ui->widget_plot_2;
    m_customPlot[2] = ui->widget_plot_3;
    QString _legendName[RADARNUM] = {"副雷达1","主雷达","副雷达2"};
    // 设置坐标范围、坐标名称
    for(auto idx = 0;idx < RADARNUM;idx++){
        m_customPlot[idx]->addGraph();
        QPen pen;
        pen.setWidth(4);
        pen.setColor(Qt::red);
        m_customPlot[idx]->graph(0)->setPen(pen);

        QFont font;
        font.setPixelSize(15);
        if(ISSHOWCONTENT == true){
            for(auto i = 0;i < RADAR_ARS408_TABLE_MAX;i++){
                m_sameTimeTextTip[idx][i] = new QCPItemText(m_customPlot[idx]);
                m_sameTimeTextTip[idx][i]->setPositionAlignment(Qt::AlignTop | Qt::AlignHCenter);
                m_sameTimeTextTip[idx][i]->position->setType(QCPItemPosition::ptAbsolute);

                m_sameTimeTextTip[idx][i]->setFont(font);
                m_sameTimeTextTip[idx][i]->setPen(QPen(Qt::black));
                m_sameTimeTextTip[idx][i]->setBrush(Qt::white);
                m_sameTimeTextTip[idx][i]->setVisible(false);
            }
        }
        
         //坐标轴使用时间刻度
         //四边安上坐标轴
         m_customPlot[idx]->axisRect()->setupFullAxesBox();
         //设置y轴范围
         m_customPlot[idx]->xAxis->setRange(-100, 100);
         m_customPlot[idx]->yAxis->setRange(-20, 300);

         // 绘制散点
         m_customPlot[idx]->graph(0)->setLineStyle(QCPGraph::lsNone);
         m_customPlot[idx]->graph(0)->setScatterStyle(QCPScatterStyle(QCPScatterStyle::ssCircle, 2));

        // 设置图例
        m_customPlot[idx]->legend->setVisible(true);//dis
        m_customPlot[idx]->axisRect()->insetLayout()->setInsetAlignment(0,Qt::AlignTop|Qt::AlignVCenter);//alignment
        m_customPlot[idx]->legend->setBrush(QColor(255, 255, 255, 150));//color
        m_customPlot[idx]->graph(0)->setName(_legendName[idx]);//name
        m_customPlot[idx]->legend->setBorderPen(Qt::NoPen);//border

        //  m_sameTimeTextTip = new QCPItemText(m_customPlot[idx]);
        //  m_sameTimeTextTip->setPositionAlignment(Qt::AlignTop|Qt::AlignHCenter);   m_sameTimeTextTip->position->setType(QCPItemPosition::ptAbsolute);

        //  QFont font;
        //  font.setPixelSize(10);
        //  m_sameTimeTextTip->setFont(font); // make font a bit larger
        //  m_sameTimeTextTip->setPen(QPen(Qt::black)); // show black border around text
        //  m_sameTimeTextTip->setBrush(Qt::white);
        //  m_sameTimeTextTip->setVisible(true);

         // 使上下轴、左右轴范围同步
         connect(m_customPlot[idx]->xAxis, SIGNAL(rangeChanged(QCPRange)), m_customPlot[idx]->xAxis2, SLOT(setRange(QCPRange)));
         connect(m_customPlot[idx]->yAxis, SIGNAL(rangeChanged(QCPRange)), m_customPlot[idx]->yAxis2, SLOT(setRange(QCPRange)));

         m_customPlot[idx]->setInteractions(QCP::iRangeDrag | QCP::iRangeZoom);
    }

}

int PlotViewer::getDevIdx(int curidx,int curdev)
{
    int idx{0};
    if(m_dev0_is_master_radar){//界面设置0号设备是主雷达
        if(curdev == 0){//0号设备有数据
            idx == 0;//0号设备放到主雷达位置
        }else{//1号设备只能放到副雷达位置
            idx = curidx+1;
        }
    }else{//界面设置1号设备是主雷达
        if(curdev == 1){//1号设备有数据
            idx == 0;//0号设备放到主雷达位置
        }else{//0号设备只能放到副雷达位置
            idx = curidx+1;
        }
    }
}

void PlotViewer::doWork()
{
    StructRadarDataPtr data;
    QVector<double> _x,_y,_id;
    int _devIdx{0};
    while(m_isRun){
        if(m_queue.wait_and_pop_fortime(data,50)){
            _devIdx = getDevIdx(data.get()->_idx,data.get()->_devID);
            
            for(auto i = 0;i < data.get()->_objNum;i++){
                // std::cout <<data.get()->_RecvTime +":"
                // << " objNum::" << data.get()->_objNum
                // << "  id:"<< data.get()->_RadarInfo[i]._id
                // << "  x:"<< data.get()->_RadarInfo[i]._x 
                // << "  y:"<< (int)(data.get()->_RadarInfo[i]._y*1000)/1000.0<< std::endl;
                _x.push_back(data.get()->_RadarInfo[i]._x );
                _y.push_back(data.get()->_RadarInfo[i]._y );
                _id.push_back(data.get()->_RadarInfo[i]._id);
            }
            dis_plot(_devIdx,_id,_x,_y);
            _x.clear(); _y.clear(); _id.clear();
        // }else{
			std::this_thread::sleep_for(std::chrono::milliseconds(50));
		}
    }
}

void PlotViewer::dis_plot(int8_t idx, QVector<double> &_id, QVector<double> &_x, QVector<double> &_y)
{
    // m_customPlot[idx]->graph(0)->setData(_x, _y);
    m_customPlot[idx]->graph(0)->setData(_y, _x);
    if(ISSHOWCONTENT == true){
        for(auto i = 0;i < RADAR_ARS408_TABLE_MAX;i++){
            m_sameTimeTextTip[idx][i]->setVisible(false);
        }
        for(auto i = 0;i < m_customPlot[idx]->graph(0)->dataCount();i++){
                if(i >= RADAR_ARS408_TABLE_MAX) continue;
                double x = m_customPlot[idx]->graph(0)->data()->at(i)->key;
                double y = m_customPlot[idx]->graph(0)->data()->at(i)->value;
                double xpos = m_customPlot[idx]->xAxis->coordToPixel(x);
                double ypos = m_customPlot[idx]->yAxis->coordToPixel(y);
                
                m_sameTimeTextTip[idx][i]->setText("("+QString::number(_id.at(i))+"-"+QString::number(x)+","+QString::number(y)+")");
                m_sameTimeTextTip[idx][i]->position->setCoords(xpos,ypos);
                m_sameTimeTextTip[idx][i]->setVisible(true);
        }
   }
   m_customPlot[idx]->replot();
}




void PlotViewer::slot_set_radar_path(QString &path)
{
    m_savePath = path;
}

void PlotViewer::radar_init(int port)
{
    if(port >=2){
        Log::Logger->warn("radar idx max num is 1,but input num is:{}",port);
        return;
    }

    if(m_radar_ars408[port] != nullptr){
        delete m_radar_ars408[port];
        m_radar_ars408[port] = nullptr;
    }
    m_radar_ars408[port] = new Radar_Ars408(static_cast<EnumCanDevIdx>(port));
    m_radar_ars408[port]->init();
    

    connect(m_radar_ars408[port], &Radar_Ars408::signal_one_frame_pro,this,&PlotViewer::slot_RadaroneFrameProc,Qt::DirectConnection);
}

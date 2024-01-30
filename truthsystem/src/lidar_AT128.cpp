#include "lidar_AT128.h"
#include <QProcess>
#include <QDebug>
#include <QMessageBox>
#include <QCoreApplication>
#include <functional>
#include <QDir>

#include <queue>
#include <mutex>
#include "configinfo.h"
#include "memcache.h"
#include "datacollector.h"
std::mutex g_mutexRidar;

void imageCallback(void *data);

bool g_playRadar = false;
bool g_recordRadar = false;

// std::queue<PointView> Lidar_AT128::m_queue;
// GQueue<PointView> Lidar_AT128::m_queue;
Lidar_AT128::Lidar_AT128(std::string ip, std::string _pcappath,QObject *parent)
{
    qRegisterMetaType<StructPointData*>("StructPointData*");
    m_ip = ip;
    m_pcapPath = _pcappath;
    initThread();
}

Lidar_AT128::~Lidar_AT128()
{
	if (m_procDataWorker){
		m_procDataWorker->setRunFlag(false);

		m_procDataThread.quit();
		m_procDataThread.wait();
	}
}

int Lidar_AT128::init()
{
    m_procDataThread.start();
    emit signal_proc_data_start();
    return 0;
}
void Lidar_AT128::initThread()
{
    m_procDataWorker = new LidarProcDataWorker(&m_queue,m_ip,m_pcapPath);
    // 播放，停止

    connect(this,&Lidar_AT128::signal_proc_data_start,m_procDataWorker,&LidarProcDataWorker::doWork);
    connect(&m_procDataThread,&QThread::finished,m_procDataWorker,&QObject::deleteLater);
    connect(m_procDataWorker,&LidarProcDataWorker::signal_one_frame_proc,this,&Lidar_AT128::slot_oneFrameProc,Qt::DirectConnection);
    m_procDataWorker->moveToThread(&m_procDataThread);
    m_procDataWorker->setRunFlag(true);
}
void Lidar_AT128::close()
{
    if(m_procDataWorker->getSaveFlag()){
        m_procDataWorker->stopSave();
    }
    m_procDataWorker->setRunFlag(false);
    m_procDataWorker->setRunFlag(false);

    m_procDataThread.quit();
    m_procDataThread.wait();
}

void Lidar_AT128::setSavePath(QString &path)
{
    if(m_procDataWorker){
        return m_procDataWorker->setSavePath(path);
    }
}

bool Lidar_AT128::startSave()
{
    if(m_procDataWorker){
        return m_procDataWorker->startSave();
    }
    return false;
}

void Lidar_AT128::stopSave()
{
    if (m_procDataWorker)
	{
		m_procDataWorker->stopSave();
	}
}

bool Lidar_AT128::getSaveFlag()
{
    if(m_procDataWorker){
        return m_procDataWorker->getSaveFlag();
    }
    return false;
}

void Lidar_AT128::stopRecv()
{
    if(m_procDataWorker)
        m_procDataWorker->stopRecv();
}

void Lidar_AT128::slot_oneFrameProc(StructPointData* data)
{
    emit signal_one_frame_pro(data);
}


void Lidar_AT128::slotPlayRadar(bool doPlay)
{
    if (doPlay) {
        this->init();
        g_playRadar = true;
    } else {
        //m_lidar_at128->close();
        g_playRadar = false;
    }
}


void Lidar_AT128::slotRecordRadar(bool doRecord)
{
    if (!g_playRadar) {
        qDebug() << "no play video to save";
        return;
    }
    if (doRecord) {
        g_recordRadar = true;
    } else {
        //m_lidar_at128->close();
        g_recordRadar = false;
    }
}

void lidar_xSeconds2Date(unsigned long seconds, unsigned int useconds, _xtime& time)
{
    static unsigned int month[12] = {
        31,
        28,
        31,
        30,
        31,
        30,
        31,
        31,
        30,
        31,
        30,
        31
    };
    unsigned int days;
    unsigned short leap_y_count;
    seconds += 8 * 60 * 60;        
    time.second = seconds % 60;
    seconds /= 60;
    time.minute = seconds % 60;
    seconds /= 60;
    time.hour = seconds % 24;
    days = seconds / 24;
    leap_y_count = (days + 365) / 1461;
    if (((days + 366) % 1461) == 0)
    {
        //��������1�� 
        time.year = 1970 + (days / 366);
        time.month = 12;              
        time.day = 31;
        return;
    }
    days -= leap_y_count;
    time.year = 1970 + (days / 365);      
    days %= 365;                      
    days = 01 + days;                 
    if ((time.year % 4) == 0)
    {
        if (days > 60)--days;            
        else
        {
            if (days == 60)
            {
                time.month = 2;
                time.day = 29;
                return;
            }
        }
    }
    for (time.month = 0;month[time.month] < days;time.month++)
    {
        days -= month[time.month];
    }
    ++time.month;               
    time.day = days;           

    time.msecond = useconds / 1000; 
}
void lidar_float2Time(double time,uint64_t &second,uint32_t &usecond)
{
    // QString str(QString::number(time,'f',6));
    std::string str=std::to_string(time);
    QStringList list=QString::fromStdString(str).split(".");
    if(list.size()>1){
        second = list.at(0).toLong();
        usecond = list.at(1).toLong();
    }
    
}

void AT128_lidarCallback(boost::shared_ptr<PPointCloud> cld, double timestamp)
{

    if (!g_playRadar) return;
    PointView frame;
   _xtime time;
   uint64_t second{0};
   uint32_t usecond{0};
   lidar_float2Time(timestamp,second,usecond);
   lidar_xSeconds2Date(second,usecond,time);
   QString _timestr = QString::asprintf("%.4d-%.2d-%.2d %.2d-%.2d-%.2d %.3d",time.year, time.month, time.day,
																		   time.hour, time.minute, time.second, time.msecond);
    
    if(cld.get()){
        frame.frameTime = _timestr;
        frame.time = timestamp;
        frame.pointsPtr = std::make_shared<pcl::PointCloud<PointXYZIT> >();
        if(frame.pointsPtr.get())
            frame.pointsPtr.get()->assign(cld.get()->begin(),cld.get()->end());
    }


    // DataCollector::instance()->add(frame);
    // Lidar_AT128::m_queue.push(frame);
    GlobalSignals::getInstance()->signal_one_frame_proc(frame);
    // qDebug() << QString("m_queue size:%1").arg(Lidar_AT128::m_queue.size());

}
void AT128_rawcallback(PandarPacketsArray *array) {
   // printf("array size: %d\n", array->size());
}

void AT128_faultmessagecallback(AT128FaultMessageInfo &faultMessage) {

}

void AT128_gpsCallback(double timestamp) {
}

LidarProcDataWorker::LidarProcDataWorker(GQueue<PointView> *queue, std::string ip,std::string _pcappath,QObject *parent)
{
    m_ip = ip;
    _queue = queue;
    m_pcapPath = _pcappath;
}

LidarProcDataWorker::~LidarProcDataWorker()
{
}
#include <strings.h>
void LidarProcDataWorker::process()
{
    std::thread::id id = std::this_thread::get_id();
    stringstream ss;
    ss << id;
    Log::Logger->info("AT128_lidarCallback::process() threadId:{}", ss.str());
    PandarSwiftSDK* spPandarSwiftSDK;
//m_pcapPath
    std::map<std::string, int32_t> configMap;
    configMap["process_thread"] = 91;
    configMap["publish_thread"] = 90;
    configMap["read_thread"] = 99;
    configMap["timestamp_num"] = 0;
    configMap["without_pointcloud_udp_warning_time"] = 10000; // ms
    configMap["without_faultmessage_udp_warning_time"] = 10000; // ms
    configMap["untragger_pclcallback_warning_time"] = 10000; // ms


    spPandarSwiftSDK = new PandarSwiftSDK(
                                std::string(m_ip),
                                std::string(""), 2368, 10110,
                                std::string("PandarAT128"), \
                                std::string("./params/corrections1.5.dat"), \
                                std::string("./params/AT128E2X_Firetime Correction File.csv"), \
                                std::string(""), AT128_lidarCallback,  AT128_rawcallback, AT128_gpsCallback, AT128_faultmessagecallback,\
                                std::string(""), \
                                std::string(""), \
                                std::string("./params/ca-chain.cert.pem"), \
                                0, 0, 1, \
                                std::string("both_point_raw"), "", configMap);

        spPandarSwiftSDK->start();
        while (!spPandarSwiftSDK->GetIsReadPcapOver()) {
            usleep(1000);
        }
        spPandarSwiftSDK->stop();
        delete spPandarSwiftSDK;
        spPandarSwiftSDK = nullptr;
}
void LidarProcDataWorker::LidarInit(std::string ip, int port)
{
    new std::thread(&LidarProcDataWorker::process, this);  
}

void LidarProcDataWorker::stopRecv()
{
    // if(spPandarSwiftSDK != nullptr){
    //     spPandarSwiftSDK->stop();
    //     delete spPandarSwiftSDK;
    //     spPandarSwiftSDK = nullptr;
    // }
}

void LidarProcDataWorker::doWork()
{
    LidarInit(m_ip,2368);
    Log::Logger->info("AT128_lidarCallback::doWork() threadId:{}",QThread::currentThreadId());
    // new std::thread([this](){
    static uint64_t frameCnt = 0;
    CSenceInfo _senceInfo;
    QTime _startTime = QTime::currentTime();
    QString _pcdPath{""};
    pcl::PCDWriter writer;
    int interval = MemCache::instance()->value("LidarSaveInterval").toInt();
    if(interval <=0 || interval > 100) interval = 10;
#if 0
    PointView data;
    while(_run && _queue != nullptr) {
        static bool flag{false};
        if(!flag){
            Log::Logger->info("AT128_lidarCallback::doWork()::while threadId:{}",QThread::currentThreadId());
            flag = true;
        }
        
        // {
        //     std::unique_lock<std::mutex> locker(g_mutexRidar);
        //     if (!_queue->empty()) {
        //         data = _queue->front();
        //         _queue->pop();
        //     }
        // }
        if (nullptr == data) continue;
        if(!_queue->wait_and_pop_fortime(data,50)){
            continue;
        }

        _saveFlag = g_recordRadar;
        if (!_saveFlag) {
            QThread::usleep(10);
            continue;
        }



        //emit signal_one_frame_proc(data); // show

        // _savePath = QCoreApplication::applicationDirPath()+ QString("/radar/");
        ConfigInfo::getInstance()->GetSavePath(_savePath);
        ConfigInfo::getInstance()->GetSenceInfo(_senceInfo);
        QString _sPath = QString("%1/At128_%2_%3_%4_%5_%6℃_%7%rh/").arg(_savePath).arg(_senceInfo._Time).arg(_senceInfo._Weather)
                                                                   .arg(_senceInfo._TimeQuantum).arg(_senceInfo._Scene).arg(_senceInfo._Temp).arg(_senceInfo._Wet);
                                                                   

        //new std::thread([this](StructPointData* x){
            if(_saveFlag && _sPath != ""){
                PPointCloud points;
                frameCnt++;
                QString _strT = _startTime.toString("ss_hh_mm");
                
                QTime _stopTime = QTime::currentTime();
                
                if(_startTime.msecsTo(_stopTime) >= 1000*60*interval){//10min
                    _startTime = _stopTime;
                    
                }
                _pcdPath = _sPath+_strT+"/";
                QDir dir;
                if(!dir.exists(_pcdPath)){
                    dir.mkpath(_pcdPath);
                    qDebug() << _pcdPath<< ">>>>>>>>>>>>>>>" << frameCnt;
                }
                for(int i = 0;i < data.pointsPtr.get()->size(); i++) {
                    PPoint pt;
                    pt.x = data.pointsPtr.get()->at(i).x;           //data->point[i].X;
                    pt.y = data.pointsPtr.get()->at(i).y;           //data->point[i].Y;
                    pt.z = data.pointsPtr.get()->at(i).z;           //data->point[i].Z;
                    pt.intensity = data.pointsPtr.get()->at(i).intensity;   //data->point[i].Intensity;
                    // pt.timestamp = data->point[i].time;
                    points.emplace_back(pt);
                    // points.emplace_back(PointXYZIT(data->point[i].X,data->point[i].Y,data->point[i].Z,data->point[i].Intensity,data->point[i].time));
                // vecPoints.push_back(QVector3D(data->point[i].X,data->point[i].Y,data->point[i].Z));
                }
                
                std::string name = _pcdPath.toStdString() + std::to_string(data.time)/*data->time*//*+ std::to_string(frameCnt)*/  + ".pcd";
                try{
                    writer.write(name, points,true);
                }catch (pcl::PCLException &e){
                    Log::Logger->error("pcl::PCLException:{}",e.what());
                }
                
            }

            // if (data) {
            //     delete data;
            //     data = nullptr;
            // }
        //}, data);
    }
    #endif
    // });
}
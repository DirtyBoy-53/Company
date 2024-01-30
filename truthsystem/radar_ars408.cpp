#include "radar_ars408.h"
#include "log.h"
#include <QDateTime>
#include <QDir>
#include "configinfo.h"
#include "memcache.h"
#include "utils/global.hpp"
#include "datacollector.h"
Radar_Ars408::Radar_Ars408(EnumCanDevIdx devidx,QObject *parent)
{
    qRegisterMetaType<StructRadarDataPtr>("StructRadarDataPtr");
    m_devIdx = static_cast<uint32_t>(devidx);
    m_usbCan_alyst = new UsbCan_alyst_II(m_devIdx);

    if(m_usbCan_alyst->usbCanInit(0)){
        SendDataToRadar();
        initThread();
    }  
}

Radar_Ars408::~Radar_Ars408()
{
    // close();
}

int Radar_Ars408::init()
{
    m_recvDataThread.start();
    emit signal_recv_data_start();

    m_procDataThread.start();
    emit signal_proc_data_start();
    return 0;
}

void Radar_Ars408::close()
{
    if (m_recvDataWorker){
		m_recvDataWorker->setRunFlag(false);
		
		m_recvDataThread.quit();
		m_recvDataThread.wait();
        m_recvDataWorker = nullptr;
	}
	if (m_procDataWorker){
		m_procDataWorker->setRunFlag(false);

		m_procDataThread.quit();
		m_procDataThread.wait();
        m_procDataWorker = nullptr;
	}

    m_usbCan_alyst->usbCanClose();
    if(m_usbCan_alyst){
        delete m_usbCan_alyst;
        m_usbCan_alyst = nullptr;
    }
    
}

bool Radar_Ars408::startSave()
{
    if(m_procDataWorker){
        m_procDataWorker->startSave();
    }
    return true;
}

void Radar_Ars408::stopSave()
{
    if(m_procDataWorker){
        m_procDataWorker->stopSave();
    }
}

bool Radar_Ars408::getSaveFlag()
{
    return false;
}

bool Radar_Ars408::SendDataToRadar()
{
    ARS_40X_CAN ars_40x_can;
    ars_40x_can.radar_cfg_.set_output_type(0x01);//设置输出object
    ars_40x_can.radar_cfg_.set_send_quality(false);
    ars_40x_can.radar_cfg_.set_send_ext_info(false);
    
    m_usbCan_alyst->sendData(m_devIdx,0x200,0,0,ars_40x_can.radar_cfg_.radar_cfg_msg.raw_data,8);
    return true;
}

void Radar_Ars408::setPath(QString &path)
{
    if(m_procDataWorker){
        m_procDataWorker->setSavePath(path);
    }
}

void Radar_Ars408::initThread()
{
    m_recvDataWorker = new RadarRecvDataWorker_Ars408(&m_queue,m_usbCan_alyst);
    connect(this,&Radar_Ars408::signal_recv_data_start,m_recvDataWorker,&RadarRecvDataWorker_Ars408::doWork);
    connect(&m_recvDataThread,&QThread::finished,m_recvDataWorker,&QObject::deleteLater);
    connect(m_recvDataWorker, &RadarRecvDataWorker_Ars408::signal_one_frame_proc,this,&Radar_Ars408::slot_oneFrameProc);
    m_recvDataWorker->moveToThread(&m_recvDataThread);
    m_recvDataWorker->setDevId(m_devIdx);
    m_recvDataWorker->setRunFlag(true);

    m_procDataWorker = new RadarProcDataWorker_Ars408(&m_queue);
    connect(this,&Radar_Ars408::signal_proc_data_start,m_procDataWorker,&RadarProcDataWorker_Ars408::doWork);
    connect(&m_procDataThread,&QThread::finished,m_procDataWorker,&QObject::deleteLater);
    m_procDataWorker->moveToThread(&m_procDataThread);
    m_procDataWorker->setDevId(m_devIdx);
    m_procDataWorker->setRunFlag(true);
    
}

void Radar_Ars408::slot_oneFrameProc(StructRadarDataPtr data)
{
    emit signal_one_frame_pro(data);
}

RadarRecvDataWorker_Ars408::RadarRecvDataWorker_Ars408(GQueue<StructRadarDataPtr>* queue, UsbCan_alyst_II *usbcan, QObject *parent)
{
    m_queue = queue;
    m_usbCan_alyst = usbcan;
}

RadarRecvDataWorker_Ars408::~RadarRecvDataWorker_Ars408()
{

}

void RadarRecvDataWorker_Ars408::doWork()
{
    VCI_CAN_OBJ buff1[RX_BUFF_SIZE]; // buffer
    VCI_CAN_OBJ buff2[RX_BUFF_SIZE]; // buffer
    ARS_40X_CAN ars_40x_can1;
    ARS_40X_CAN ars_40x_can2;
    int recvCnt1{0};
    int recvCnt2{0};
    int ObjCnt1{0};
    int ObjCnt2{0};

    StructRadarDataPtr frame1{nullptr};
    StructRadarDataPtr frame2{nullptr};
    while(m_isRunflag){
        memset(buff1,0,sizeof(buff1));
        memset(buff2,0,sizeof(buff2));
        uint32_t RecvFrame1 = m_usbCan_alyst->reciveData(buff1,RX_BUFF_SIZE,0);
        uint32_t RecvFrame2 = m_usbCan_alyst->reciveData(buff2,RX_BUFF_SIZE,1);
        if (!RecvFrame1 && !RecvFrame2)
            continue;

        // if(RecvFrame1 >= RADAR_ARS408_OBJNUM_MAX){
        //     Log::Logger->error("can recv msg size is {},max is {}.error.",RecvFrame1,RADAR_ARS408_OBJNUM_MAX);
        //     continue;
        // }
        
        // if(RecvFrame2 >= RADAR_ARS408_OBJNUM_MAX){
        //     Log::Logger->error("can recv msg size is {},max is {}.error.",RecvFrame2,RADAR_ARS408_OBJNUM_MAX);
        //     continue;
        // }

        for(auto i = 0;i < RecvFrame1;i++){
            //parse can data
            ars_40x_can1.receive_radar_data(buff1[i].ID,buff1[i].Data,buff1[i].DataLen);

            if(buff1[i].ID == static_cast<uint32_t>(can_messages::Object_0_Status)){
                    ObjCnt1 = ars_40x_can1.object_0_status_.get_no_of_objects();
                    std::cout << "RecvObjNum:" << ObjCnt1 << std::endl;
                    frame1 = std::make_shared<StructRadarData>();
                    frame1.get()->_idx = 0;
                    frame1.get()->_devID = m_devIdx;
                    // frame1.get()->_RecvTime =QDateTime::currentDateTime().toString("hh-mm-ss zzz");
                    frame1.get()->_RecvTime = buff1[i].TimeStamp;
                    frame1.get()->_UnixTime = YDateTime::currentUnixTime();

                    frame1.get()->_objNum = ObjCnt1;
                    recvCnt1 = 0;
            }
            if(ObjCnt1 > 0){
                // for(auto i = 0;i < RecvFrame1;i++){
                    //get obj id,x,y
                    if(buff1[i].ID == static_cast<uint32_t>(can_messages::Object_1_General)){
                        
                        // std::cout << "CurObjIdx:" << recvCnt1 << std::endl;
                        frame1.get()->_RadarInfo[recvCnt1]._id = ars_40x_can1.object_1_general_.get_object_id();//ID
                        frame1.get()->_RadarInfo[recvCnt1]._x = ars_40x_can1.object_1_general_.get_object_long_dist();//X
                        frame1.get()->_RadarInfo[recvCnt1]._y = ars_40x_can1.object_1_general_.get_object_lat_dist();//Y
                        frame1.get()->_RadarInfo[recvCnt1]._xVel = ars_40x_can1.object_1_general_.get_object_long_rel_vel();//x_Vel
                        frame1.get()->_RadarInfo[recvCnt1]._yVel = ars_40x_can1.object_1_general_.get_object_lat_rel_vel();//y_Vel
                        recvCnt1++;
                        if(recvCnt1 >= RADAR_ARS408_OBJNUM_MAX) break;
                        if(recvCnt1 >= ObjCnt1){
                            DataCollector::instance()->add(frame1);
                            // qDebug() << QString("Dev%1:RecvFrameCnt:%2 SendFrameCnt:%3").arg(1).arg(RecvFrame1).arg()
                            emit signal_one_frame_proc(frame1);
                            m_queue->push(frame1);
                            ObjCnt1 = 0;
                        }
                    }    
                // }
            }
        }




        for(auto i = 0;i < RecvFrame2;i++){
            //parse can data
            ars_40x_can2.receive_radar_data(buff2[i].ID,buff2[i].Data,buff2[i].DataLen);

            if(buff2[i].ID == static_cast<uint32_t>(can_messages::Object_0_Status)){
                    ObjCnt2 = ars_40x_can2.object_0_status_.get_no_of_objects();
                    // std::cout << "RecvObjNum:" << ObjCnt2 << std::endl;
                    frame2 = std::make_shared<StructRadarData>();
                    frame2.get()->_idx = 0;
                    frame2.get()->_devID = m_devIdx;
                    // frame2.get()->_RecvTime =QDateTime::currentDateTime().toString("hh-mm-ss zzz");
                    frame2.get()->_RecvTime = buff2[i].TimeStamp;
                    frame2.get()->_UnixTime = YDateTime::currentUnixTime();
                    frame2.get()->_objNum = ObjCnt2;
                    recvCnt2 = 0;
            }
            if(ObjCnt2 > 0){
                // for(auto i = 0;i < RecvFrame1;i++){
                    //get obj id,x,y
                    if(buff2[i].ID == static_cast<uint32_t>(can_messages::Object_1_General)){
                        
                        std::cout << "CurObjIdx:" << recvCnt1 << std::endl;
                        frame2.get()->_RadarInfo[recvCnt1]._id = ars_40x_can2.object_1_general_.get_object_id();//ID
                        frame2.get()->_RadarInfo[recvCnt1]._x = ars_40x_can2.object_1_general_.get_object_long_dist();//X
                        frame2.get()->_RadarInfo[recvCnt1]._y = ars_40x_can2.object_1_general_.get_object_lat_dist();//Y
                        frame2.get()->_RadarInfo[recvCnt1]._xVel = ars_40x_can2.object_1_general_.get_object_long_rel_vel();//x_Vel
                        frame2.get()->_RadarInfo[recvCnt1]._yVel = ars_40x_can2.object_1_general_.get_object_lat_rel_vel();//y_Vel
                        recvCnt2++;
                        if(recvCnt2 >= RADAR_ARS408_OBJNUM_MAX) break;
                        if(recvCnt2 >= ObjCnt2){
                            emit signal_one_frame_proc(frame2);
                            m_queue->push(frame2);
                            ObjCnt2 = 0;
                        }
                    }    
                // }
            }
        } 
       
    }
}

RadarProcDataWorker_Ars408::RadarProcDataWorker_Ars408(GQueue<StructRadarDataPtr> *queue, QObject *parent)
{
    m_queue = queue;
}

RadarProcDataWorker_Ars408::~RadarProcDataWorker_Ars408()
{
}

bool RadarProcDataWorker_Ars408::startSave()
{
    _saveFlag = true;
    return true;
}

void RadarProcDataWorker_Ars408::stopSave()
{
    _saveFlag = false;
}
#include "utils/global.hpp"
void RadarProcDataWorker_Ars408::doWork()
{
    StructRadarDataPtr data{nullptr};
    static uint64_t frameCnt = 0;
    QDir dir;
    QString content1;
    QString content2;
    if(_savePath.size() <= 0) {
        _savePath = "/media/xy/raid0";
    }
    QString head = "time_ns,track_id,velocity_x,velocity_y,position_x,position_y";
    static QTime _startTime = QTime::currentTime();
    QString currentTime = YDateTime::currentTime();
    QString _strT{""};
    QString _csvPath{""};
    QString _lastFilename{""};
    uint32_t _writeCnt{0}; 
    CSenceInfo _senceInfo;
    int interval = MemCache::instance()->value("RadarSaveInterval").toInt();
    if(interval <=0 || interval > 100) interval = 10;
    while(_run && m_queue != nullptr){
        if(!m_queue->wait_and_pop_fortime(data,100)){
            continue;
        }

        auto frame = data.get();
    
        if(_saveFlag){
            ConfigInfo::getInstance()->GetSavePath(_savePath);
            ConfigInfo::getInstance()->GetSenceInfo(_senceInfo);
            QString _sPath = QString("%1/ARS408_%2_%3_%4_%6℃_%7%rh/").arg(_savePath).arg(_senceInfo._Time).arg(_senceInfo._Weather)
                                                                   .arg(_senceInfo._TimeQuantum).arg(_senceInfo._Temp).arg(_senceInfo._Wet);

            QString sFilePre = QString("%1%2/%3/").arg(_sPath).arg(_senceInfo._Scene).arg(YDateTime::currentDate());
            // QString sFilePre = _sPath + YDateTime::currentDate() + "/";
            
            // _strT = _startTime.toString("hh_mm_ss_");
            QTime _stopTime = QTime::currentTime();
            if(_startTime.msecsTo(_stopTime) >= 1000*60*interval){//10min
                _startTime = _stopTime;
                currentTime = YDateTime::currentTime();
                if(m_CSV.OpenFlag){
                    m_CSV.CloseFile();
                }
                _writeCnt++;
            }
            QDir dir;
            if(!dir.exists(sFilePre)){
                dir.mkpath(sFilePre);
            }
            QString _filename = sFilePre +  currentTime +
                            QString("_Dev%1_ID%2_Cnt%3.csv").arg(m_devIdx+1).arg(frame->_idx+1).arg(_writeCnt);

            //save data
            if(!m_CSV.OpenFlag)
                m_CSV.OpenFile(_filename,head);

            for(auto i =0;i < frame->_objNum;i++ ){
                content1 = QString("%1,%2,%3,%4,%5,%6").arg(frame->_UnixTime).arg(frame->_RadarInfo[i]._id)//recv_time object_id
                                                        .arg(frame->_RadarInfo[i]._xVel).arg(frame->_RadarInfo[i]._yVel)//xVel yVel
                                                        .arg(frame->_RadarInfo[i]._x).arg(frame->_RadarInfo[i]._y);//xDis  yDis
                m_CSV.WriteToCsv(content1);
            }
            
        }else{
            _startTime = QTime::currentTime();
            if(m_CSV.OpenFlag){
                m_CSV.CloseFile();
                _writeCnt=0;
            }
        }
    }
    if(m_CSV.OpenFlag){
        m_CSV.CloseFile();
    }
}
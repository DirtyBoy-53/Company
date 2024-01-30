#include "lidarviewerwidget.h"
#include <QFileDialog>
#include <QMessageBox>
#include "configinfo.h"
#include "memcache.h"
LidarViewerWidget::LidarViewerWidget(QWidget *parent)
	: QWidget(parent)
{
	ui.setupUi(this);
	qRegisterMetaType<PointView>("PointView");
	connect(this, &LidarViewerWidget::sig_update_lidar_data, ui.pointViewerWidget, &Viewer::updateViewMaster);
	connect(this, &LidarViewerWidget::sig_update_lidar_timestamp, ui.pointViewerWidget, &Viewer::updateViewTime);
	connect(GlobalSignals::getInstance(), &GlobalSignals::signal_one_frame_proc, this, &LidarViewerWidget::slot_recv_lidar_frame);
	// connect(GlobalSignals::getInstance(), &GlobalSignals::signal_one_frame_proc, ui.pointViewerWidget, &Viewer::updateViewMaster);


}

LidarViewerWidget::~LidarViewerWidget()
{
	m_isRun = false;
	if(m_lidarConWidget){
		m_lidarConWidget->hide();
		delete m_lidarConWidget;
		m_lidarConWidget = nullptr;
	}
}

void LidarViewerWidget::readPcdFile(const std::string& file_path, std::vector<QVector4D>& cloud)
{

	std::ifstream infile;
	infile.open(file_path);
	assert(infile.is_open());
	std::string s;
	float x, y, z, i;
	while (std::getline(infile, s)) {
		std::stringstream ss(s);
		ss >> x;
		ss >> y;
		ss >> z;
		ss >> i;
		//qDebug() << QString("point_data:%1,%2,%3").arg(_pointData.x).arg(_pointData.y).arg(_pointData.z);
		cloud.emplace_back(QVector4D(x, y, z, i));
	}
	infile.close();

}

void LidarViewerWidget::LoadPcdFolder()
{
	QString path{""};
	ConfigInfo::getInstance()->GetSavePath(path);
	QString fileName = QFileDialog::getExistingDirectory(
		this, tr("select the lidar data folder."), path);
	if (fileName.isEmpty()) return;
	m_isShow_local_lidar_data = false;
	m_isShow_net_lidar_data = false;
	m_pcdFileList.clear();

	QDir dir(fileName);
	QStringList _allfile = dir.entryList(QStringList() << "*.pcd");
	for (auto data : _allfile) {
		m_pcdFileList.append(fileName + "/" + data);
	}
	if (m_pcdFileList.size() > 0) {
		m_isShow_local_lidar_data = true;
		m_pcd_cur_pos = 0;
	}
	if (m_isRun == false) {
		m_isRun = true;
		new std::thread(&LidarViewerWidget::doWork, this);
	}
}

void LidarViewerWidget::slot_btn_clicked_tools() {
	QString _Objname = ((QPushButton*)sender())->objectName();
	if (_Objname == "btn_connect") {
		m_isShow_local_lidar_data = false;

		m_isShow_net_lidar_data = true;
		if (m_isRun == false) {
			m_isRun = true;
			new std::thread(&LidarViewerWidget::doWork, this);
		}
		if(m_lidarConWidget == nullptr){
			m_lidarConWidget = new LidarConnectWidget();
			connect(m_lidarConWidget,&LidarConnectWidget::sig_lidar_init,this,&LidarViewerWidget::slot_lidar_init);
			connect(m_lidarConWidget,&LidarConnectWidget::sig_lidar_disconnect,this,&LidarViewerWidget::slot_lidar_disconnect);
		}
		m_lidarConWidget->show();	
	} else if(_Objname == "btn_opendir") {
		if(m_lidarConWidget){
			m_lidarConWidget->close();
			delete m_lidarConWidget;
			m_lidarConWidget = nullptr;
		}
		if(m_lidar_at128){
			m_lidar_at128->close();
			m_isShow_net_lidar_data = false;
		}
		LoadPcdFolder();
	} else if (_Objname == "btn_front") {
		emit sig_set_camera_view(ENUM_VIEW::Enum_front);
	} else if (_Objname == "btn_right") {
		emit sig_set_camera_view(ENUM_VIEW::Enum_right);
	} else if(_Objname == "btn_top") {
		emit sig_set_camera_view(ENUM_VIEW::Enum_top);
	} else if (_Objname == "btn_projection") {
		static bool _isShowPro{ false };
		if (_isShowPro == false) {
			_isShowPro = true;
			emit sig_set_projection_mode(Camera::PERSPECTIVE);
			ui.btn_projection->setIcon(QIcon(":/image/perspective.png"));
		} else {
			_isShowPro = false;
			emit sig_set_projection_mode(Camera::ORTHOGRAPHIC);
			ui.btn_projection->setIcon(QIcon(":/image/orthographic.png"));
		}
	} else if (_Objname == "btn_grid") {
		static bool _isShowGrid{ false };
		_isShowGrid = !_isShowGrid;
		emit sig_set_isShow_grid(_isShowGrid);
	} else if (_Objname == "btn_polar") {
		static bool _isShowPolar{ false };
		_isShowPolar = !_isShowPolar;
		emit sig_set_isShow_polar(_isShowPolar);
	} else if (_Objname == "btn_isShow_coordinate") {
		static bool _isShowCoord{ false };
		_isShowCoord = !_isShowCoord;
		emit sig_set_isShow_coordinate(_isShowCoord);
	} else {}
 

	
}

void LidarViewerWidget::slot_btn_clicked_playCtrl()
{
	QString _Objname = ((QPushButton*)sender())->objectName();
	if (_Objname == "btn_play_stop") {
		static bool _playflag{false};
		_playflag = !_playflag;
		if(_playflag){	
			ui.btn_play_stop->setIcon(QIcon(":/image/play_pause.png"));
		} else{
			ui.btn_play_stop->setIcon(QIcon(":/image/play_play.png"));
		} 
		if(m_lidar_at128)  	m_lidar_at128->slotPlayRadar(_playflag);
		else QMessageBox::information(this, "Tips", "please connect lidar by IP.");
	}else if(_Objname == "btn_record"){
		static bool _recordflag{false};
		_recordflag = !_recordflag;
		
		if(_recordflag){
			if(m_isRunSave == false){
				m_isRunSave = true;
				new std::thread(&LidarViewerWidget::saveProcess, this);
			}
		}else{
			m_isStartRecv = false;
			m_isRunSave = false;
		}
		ui.pointViewerWidget->set_is_show_recordTips(_recordflag);
	}
}


void LidarViewerWidget::slot_recv_lidar_frame(PointView data)
{
	if(m_isStartRecv){
		if(data.pointsPtr.get())
			m_SaveQueue.push(data);
		// qDebug() << QString("m_SaveQueue size:%1").arg(m_SaveQueue.size());
	}
	ui.pointViewerWidget->updateViewMaster(data);

}

void LidarViewerWidget::slot_set_lidar_path(QString &path)
{
	m_savePath = path;
}

void LidarViewerWidget::slot_lidar_init(QString ip)
{
	if(m_lidar_at128){
		delete m_lidar_at128;
		m_lidar_at128 = nullptr;
	}
	if(m_lidar_at128==nullptr){
		std::string _path = m_savePath.toStdString()+"Pcap/";
		m_lidar_at128 = new Lidar_AT128(ip.toStdString(),_path);
	}
	m_lidarConWidget->close();
	QMessageBox::information(this, "Tips", "lidar init success.");
}

void LidarViewerWidget::slot_lidar_disconnect()
{
	m_lidar_at128->stopRecv();
	QMessageBox::information(this, "Tips", "lidar disconnect success.");
}

// float g_pcd_data[AT128_DATASIZE_MAX*7]{0.0f};
void load_binary_pcdfile(const QString& filename,PointView& pt){

	QFile file(filename);
	if(!file.open(QIODevice::ReadOnly)){
		qDebug() << "Error:Can`t open pcd file:" << filename;
		return;
	}

#define POINT_SIZE 26
	// memset(g_pcd_data,0,sizeof(g_pcd_data));
	QByteArray head = file.read(217);
	QByteArray data = file.read(AT128_DATASIZE_MAX*26);
	char *buf = data.data();

	// file.read((char*)g_pcd_data,sizeof(g_pcd_data));
	float xyzi[4]{0.0f};
	PPoint ppoint;
	pt.frameTime = filename.section('/',-1);
	uint32_t cnt{0};
	for(uint32_t i = 0;i < AT128_DATASIZE_MAX;i++){
		memcpy(&ppoint.x,&buf[cnt+sizeof(float)*0],sizeof(float));
		memcpy(&ppoint.y,&buf[cnt+sizeof(float)*1],sizeof(float));
		memcpy(&ppoint.z,&buf[cnt+sizeof(float)*2],sizeof(float));
		memcpy(&ppoint.intensity,&buf[cnt+sizeof(float)*3],sizeof(float));

		cnt += POINT_SIZE;
		pt.pointsPtr.get()->emplace_back(ppoint);
	}
	
	// while(!file.atEnd()){
	// 	file.read((char*)xyzi,sizeof(xyzi));
	// 	cnt += sizeof(xyzi);
	// 	file.seek(cnt+10);
	// 	// QByteArray tr = file.read(10);
	// 	ppoint.x = xyzi[0];
	// 	ppoint.y = xyzi[1];
	// 	ppoint.z = xyzi[2];
	// 	ppoint.intensity = xyzi[3];
	// 	pt.pointsPtr.get()->emplace_back(ppoint);
	// }
	// memcpy(pts,buf+217,len-217);
	// pt.frameTime = filename.section('/',-1);
	// PPoint ppoint;
	// for(auto i = 0;i < AT128_DATASIZE_MAX;i=i+POINT_SIZE){
		
	// 	ppoint.x = pts[i];
	// 	ppoint.y = pts[i+4];
	// 	ppoint.z = pts[i+8];
	// 	ppoint.intensity = pts[i+12];
	// 	pt.pointsPtr.get()->emplace_back(ppoint);
	// }
	// delete pts;
	// pts = nullptr;
#undef POINT_SIZE
if(file.isOpen())
	file.close();
}



#define LOAD_PCDFILE_MAXNUM	20	//加载PCD文件最大数量	
void LidarViewerWidget::doWork()
{
	QStringList _pcdfilelist;
	std::vector<QVector4D> _vecPoints;
	pcl::PCDReader reader;
	
	Log::Logger->info("LidarViewerWidget::doWork()");
	auto dis_count{0};
	while (m_isRun) {
		#pragma region 本地数据
		if (m_isShow_local_lidar_data) {
#if 0
			//如果队列数据少，就通过多线程添加数据，单次不能加载超过5个文件
			if(m_ReadQueue.size() < 10){
				for(auto i = 0;m_pcd_cur_pos < m_pcdFileList.size() && i < LOAD_PCDFILE_MAXNUM;m_pcd_cur_pos++,i++){
					auto path = m_pcdFileList.at(m_pcd_cur_pos);

					new std::thread(&LidarViewerWidget::readProcess,this,path);
					// std::this_thread::sleep_for(std::chrono::milliseconds(10));
					
				}
			}

			if(!m_ReadQueue.wait_and_pop_fortime(data,100)){	
				continue;
			}
			emit sig_update_lidar_data(data);
			std::this_thread::sleep_for(std::chrono::milliseconds(500));
			qDebug() << "m_pcdFileList.size:"<<m_pcdFileList.size()<<"   dis_count" << dis_count++;
#endif
			for(auto path : m_pcdFileList){
				// QTime time;
				// time.start(); //开始计时，以ms为单位
				PointView data;
				load_binary_pcdfile(path,data);
				if(data.pointsPtr.get())
					emit sig_update_lidar_data(data);
				// qDebug() << "load_binary_pcdfile:" << time.elapsed() << "ms";
				// std::this_thread::sleep_for(std::chrono::milliseconds(10));
			}
			qDebug() << "play end.";
			m_isShow_local_lidar_data = false;
		}
		#pragma endregion
		#if 0
		#pragma region 设备数据
		else if(m_isShow_net_lidar_data){
			PointView data;
			if(Lidar_AT128::m_queue.size() > 3 && !Lidar_AT128::m_queue.wait_and_pop_fortime(data,200)){
				continue;
			}
			if(m_isStartRecv){
				if(data.pointsPtr.get())
					m_SaveQueue.push(data);
				// qDebug() << QString("m_SaveQueue size:%1").arg(m_SaveQueue.size());
			}
				
			if(data.pointsPtr.get())
				ui.pointViewerWidget->updateViewMaster(data);
			std::this_thread::sleep_for(std::chrono::milliseconds(50));
		}
		#pragma endregion
		#endif
		std::this_thread::sleep_for(std::chrono::milliseconds(100));
	}
}
void LidarViewerWidget::readProcess(QString path){

	pcl::PCDReader reader;
	std::shared_ptr<pcl::PointCloud<PointXYZIT>> cld = std::make_shared<pcl::PointCloud<PointXYZIT>>();
	reader.read(path.toStdString(),*cld,0);
	QString str = path.section('/',-1);
	PointView pv;
	pv.pointsPtr = cld;
	pv.frameTime = str;
	m_ReadQueue.push(pv);

}

void writePCD(PointView data,string name)
{
	// QFile file(QString::fromStdString(name));
	// file.open(QIODevice::WriteOnly);
	// file.write(g_pcdfile_head);
	// QDataStream aStream(&file);

	// float *pts = new float(AT128_DATASIZE_MAX * sizeof(float)*3);
	// auto size = data.pointsPtr.get()->size();
	// float pt[4]{0.0f};
	// char pPt[sizeof(float)*4];
	// for(auto i = 0;i < size;i++){
	// 	aStream << data.pointsPtr.get()->at(i).x
	// 			<< data.pointsPtr.get()->at(i).y
	// 			<< data.pointsPtr.get()->at(i).z
	// 			<< data.pointsPtr.get()->at(i).intensity;
				
	// 	// pt[0] = data.pointsPtr.get()->at(i).x;
	// 	// pt[1] = data.pointsPtr.get()->at(i).y;
	// 	// pt[2] = data.pointsPtr.get()->at(i).z;
	// 	// pt[3] = data.pointsPtr.get()->at(i).intensity;
	// 	// file.write((char*)pt);
	// }
	// if(file.isOpen()){
	// 	file.close();
	// }
	pcl::PCDWriter writer;

	// qDebug() <<"savePath:" << QString::fromStdString(name);
	try{
		// writer.write(name, points,true);
		writer.write(name,*data.pointsPtr.get(),true);

	}catch (pcl::PCLException &e){
		Log::Logger->error("pcl::PCLException:{}",e.what());
	}
}
#include "utils/global.hpp"
void LidarViewerWidget::saveProcess(){
	float x{ 0.0f }, y{ 0.0f }, z{ 0.0f }, intensity{ 0.0f };
	bool _hasData{false};
	static uint64_t frameCnt = 0;
    CSenceInfo _senceInfo;
    QTime _startTime = QTime::currentTime();
	QString currentTimeStr = YDateTime::currentTime();
    QString _pcdPath{""};
    pcl::PCDWriter writer;
    int interval = MemCache::instance()->value("LidarSaveInterval").toInt();
    if(interval <=0 || interval > 100) interval = 10;
	QString _savePath{""};
	m_isStartRecv = true;
	while(m_isRunSave){
		PointView data;
		if(!m_SaveQueue.wait_and_pop_fortime(data,100)){
				
				continue;
		}
		

		ConfigInfo::getInstance()->GetSavePath(_savePath);
        ConfigInfo::getInstance()->GetSenceInfo(_senceInfo);
        QString _sPath = QString("%1/At128_%2_%3_%4_%6℃_%7%rh/").arg(_savePath).arg(_senceInfo._Time=="" ? YDateTime::currentDate() : _senceInfo._Time)
																	.arg(_senceInfo._Weather).arg(_senceInfo._TimeQuantum).arg(_senceInfo._Temp).arg(_senceInfo._Wet);
		QString sFilePre = QString("%1%2/").arg(_sPath).arg(_senceInfo._Scene);
            if( sFilePre != ""){
                PPointCloud points;
                frameCnt++;
                
                QTime _stopTime = QTime::currentTime();
                
                if(_startTime.msecsTo(_stopTime) >= 1000*60*interval){//10min
                    _startTime = _stopTime;
					currentTimeStr = YDateTime::currentTime();
                }
                _pcdPath = sFilePre+currentTimeStr+"/";
                QDir dir;
                if(!dir.exists(_pcdPath)){
                    dir.mkpath(_pcdPath);
                    qDebug() << _pcdPath<< ">>>>>>>>>>>>>>>" << frameCnt;
                }
                
                std::string name = _pcdPath.toStdString() + std::to_string(data.time) + ".pcd";
				// new std::thread(writePCD,data,name);
				// std::this_thread::sleep_for(std::chrono::milliseconds(50));
				writePCD(data,name);
				
			}
	}
	PointView data;
	while(!m_SaveQueue.empty()){
		m_SaveQueue.wait_and_pop_fortime(data,10);
	}
		
	Log::Logger->info("LidarViewerWidget::saveProcess() exit.");
}
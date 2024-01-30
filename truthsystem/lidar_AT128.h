#ifndef LIDAR_AT128_H
#define LIDAR_AT128_H

#include <iostream>
#include <QObject>
#include <QThread>
#include <QDateTime>
#include <mutex>
#include "utils/GQueue.h"
#include "common.h"
// #include "pandersdk.h"
#include "pandarSwiftSDK.h"
#include <thread>
#include <queue>
#include "utils/cCSV.hpp"

class LidarProcDataWorker;
class Lidar_AT128 : public QObject
{
	Q_OBJECT //
private:
	QThread m_procDataThread;
	LidarProcDataWorker *m_procDataWorker{nullptr};

public:
	Lidar_AT128(std::string ip,std::string _pcappath, QObject* parent=NULL);
	~Lidar_AT128();
	int init();
	void close();
	void setSavePath(QString &path);
	bool startSave();
	void stopSave();
    bool getSaveFlag();
	void stopRecv();
// static std::queue<PointView> m_queue;
// static GQueue<PointView> m_queue;
	GQueue<PointView> m_queue;
private:
    
    void initThread();
	std::string m_ip;
	std::string m_pcapPath;
signals:
	void signal_proc_data_start();

    void signal_one_frame_pro(StructPointData* data);

public slots:
    void slot_oneFrameProc(StructPointData* data);
	void slotPlayRadar(bool doPlay);
	void slotRecordRadar(bool doRecord);
};

class LidarProcDataWorker : public QObject
{
	Q_OBJECT
public:
	LidarProcDataWorker(GQueue<PointView> *queue,std::string ip, std::string _pcappath,QObject* parent = NULL);
	~LidarProcDataWorker();
	void setRunFlag(bool flag) { _run = flag; }
	bool getRunFlag() { return _run; }
	bool getSaveFlag() { return _saveFlag; }
	void setSavePath(const QString& path) { _savePath = path; }
	bool startSave(){return _saveFlag;}
	void stopSave(){_saveFlag = false;}	
	void LidarInit(std::string ip,int port);

	void stopRecv();
private:
	GQueue<PointView>* _queue = nullptr;
	bool _run{true};
	QString _savePath = "";
	bool _saveFlag;
	std::string m_ip;
	std::string m_pcapPath;
	// PandarSwiftSDK* spPandarSwiftSDK{nullptr};

	void process();
signals:
	void signal_one_frame_proc(StructPointData* data);

public slots:
	void doWork();
};

#endif
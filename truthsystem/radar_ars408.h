#ifndef RADAR_ARS_H
#define RADAR_ARS_H

#include <iostream>
#include <QObject>
#include <QThread>
#include <QDateTime>
#include <mutex>
#include "utils/GQueue.h"
#include "common.h"
// #include "usbcan_100u.hpp"
#include "usbcan_alyst-II.hpp"
#include "utils/cCSV.hpp"
class RadarRecvDataWorker_Ars408;
class RadarProcDataWorker_Ars408;

class Radar_Ars408 : public QObject
{
	Q_OBJECT //
private:
	QThread m_recvDataThread;
	QThread m_procDataThread;

	RadarRecvDataWorker_Ars408 *m_recvDataWorker{nullptr};
	RadarProcDataWorker_Ars408 *m_procDataWorker{nullptr};
    GQueue<StructRadarDataPtr> m_queue;
    // UsbCan_100U *m_usbCan_100u{nullptr};
	UsbCan_alyst_II *m_usbCan_alyst{nullptr};
	uint32_t m_devIdx{0};
public:
	Radar_Ars408(EnumCanDevIdx devidx,QObject* parent=NULL);
	~Radar_Ars408();
	int init();
	void close();
	bool startSave();
	void stopSave();
    bool getSaveFlag();
	bool SendDataToRadar();
	void setPath(QString &path);
private:
    void initThread();
signals:
	void signal_recv_data_start();
	void signal_proc_data_start();

    void signal_one_frame_pro(StructRadarDataPtr data);

public slots:
    void slot_oneFrameProc(StructRadarDataPtr data);
};

class RadarRecvDataWorker_Ars408 : public QObject
{
	Q_OBJECT

public:
	RadarRecvDataWorker_Ars408(GQueue<StructRadarDataPtr>* queue,UsbCan_alyst_II *usbcan,QObject* parent = NULL);
	~RadarRecvDataWorker_Ars408();
	void setRunFlag(bool flag) { m_isRunflag = flag; }
	bool getRunFlag() { return m_isRunflag; }
	// void setSaveFlag(bool flag) { _saveFlag = flag; }
	// bool getSaveFlag() { return _saveFlag; }
	void setSavePath(const QString& path) { _savePath = path; }
	bool getFrameState() { return m_frameState; }
    // void startSave();
	// void stopSave();
    void init();
	void setDevId(uint32_t id){m_devIdx = id;}	
private:
	bool m_isRunflag{false};

	int64 _frameNo = 0;
	QDateTime _connTime;
	bool m_frameState = false;
	QString _savePath = "";
    GQueue<StructRadarDataPtr>* m_queue{nullptr};
    UsbCan_alyst_II *m_usbCan_alyst{nullptr};
    uint32_t m_devIdx{0}; 
signals:
    void signal_one_frame_proc(StructRadarDataPtr data);
public slots:
	void doWork();
};

class RadarProcDataWorker_Ars408 : public QObject
{
	Q_OBJECT

public:
	RadarProcDataWorker_Ars408(GQueue<StructRadarDataPtr>* queue, QObject* parent = NULL);
	~RadarProcDataWorker_Ars408();
	void setRunFlag(bool flag) { _run = flag; }
	bool getRunFlag() { return _run; }
	bool getSaveFlag() { return _saveFlag; }
	void setSavePath(const QString& path) { _savePath = path; }
	bool startSave();
	void stopSave();
	void setDevId(uint32_t id){m_devIdx = id;}	

private:
	GQueue<StructRadarDataPtr>* m_queue{nullptr};
	bool _run;
	QString _savePath = "";
	bool _saveFlag{false};
	cCSV m_CSV;
	std::mutex _writerLock;
	uint32_t m_devIdx{0}; 
public slots:
	void doWork();
};

#endif
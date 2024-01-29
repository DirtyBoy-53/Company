#ifndef DUSTIMAGEDETECTION_H
#define DUSTIMAGEDETECTION_H

#include <QQueue>
#include "baseproduce.h"
#include "XyScanDialog.h"
#include "mescom.h"
#include "comguidecfg.h"
#include "WebSocketClientManager.h"
#include "mcuserialmanager.h"
#include "dustimagefunction.h"
#include "HttpClient.h"

class DustImageDetection : public BaseProduce
{
    Q_OBJECT
public:
    explicit DustImageDetection();
    ~DustImageDetection();


private:
    void do_init();
    void do_work();
    void do_finish();
    void finish_step1();
    void finish_step2();
    void ng_step1();
    void ng_step2();
    void stop_work();

    void addLog(QString log, const int result=0);
    void show_picture(int idx);
    void save_csv_report();
    void updateTable(QString name, qint64 workTime, bool result);

    bool printer_work();
    bool work_compare_sn();

    bool mcu_power_on();
    bool mcu_power_off();

    bool get_sensor_sn();
    void get_sn();
    void mes_upload();

    bool check_video();
    bool check_shutter();
    bool check_video_auto();
    bool check_video_manual();


    bool dutSaveImage(QString name, QByteArray data);
    void getBit8FromY16(QByteArray src, unsigned char *&dest);

    bool check_register();
    bool get_register_data_com();
    bool get_register_data_iic();
    bool dut_get_register_com(int addr, int &data);
    bool dut_get_register_iic(int page, int addr, int &data);

    bool getAutoBarcode(QString &sn);
private:
    MesCom mMesObj;
    WebSocketClientManager* m_webSocket{nullptr};
    QList<GuideCfgInfo> m_tipsList;
    QString m_imgPath{""};
    QString m_ipAddress;

    QQueue<QByteArray> m_videoQueue;
    int m_frameCount{0};

    McuSerialManager* m_mcuSerial{nullptr};
    QString m_mcuCom{""};
    QString m_machineCom{""};

    DustImageFunction m_dustImageOpt;

    QString m_sensorCode{""};

    int m_seqImage = 0;
    QString m_logPath;
    QString m_completeSn{""};

    int m_dataHssd{0};
    int m_dataRaselLow{0};
    int m_dataRaselHigh{0};
    int m_dataRasel{0};

    CSerialDirect *m_handleScanner{nullptr}; // 用时开
    QString m_portScanner = "";
    // BaseProduce interface
public slots:
    virtual void slotConnectBtnClicked() override;
    virtual void slotStartWorkBtnClicked() override;
    virtual void slotStopWorkBtnClicked() override;

    void slotRecvVideoFrameWs(QByteArray frameData, FrameInfo_S frameInfo);

};

#endif // DUSTIMAGEDETECTION_H

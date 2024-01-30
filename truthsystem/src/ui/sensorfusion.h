#pragma once

#include <QWidget>
#include <QString>
#include <QTimer>
#include <QLabel>
#include <list>
#include <QPushButton>
#include <QProcess>
#define PLAYF_REQUENCY      100             //25ms
#define WAIT_FUSION_TIME    (1000*60*60*10)    //10hour
class SensorFusion : public QWidget {
    Q_OBJECT
public:
    SensorFusion(QWidget* parent = nullptr);
    ~SensorFusion();

protected:
    void initUI();
    void initConnect();

private:
    QString m_play_path{ "" };
    QTimer m_play_timer;
    std::list<QString> m_pic_vector;

    QLabel* label_image{ nullptr };
    QPushButton* btn_play_stop{nullptr};
    
    bool m_run{false};
    

    QString choose_path();
    void sendMsgByConsole();
private slots:
    void slot_play_timer();

};
#ifndef PLAYBACKUI_H
#define PLAYBACKUI_H

#include <QWidget>
#include <QFile>
#include <constdef.h>
#include <videoinfo.h>
#include <nvshandler.h>
#include <thread>
#include "configinfo.h"
#include "yuvplay.h"

#define STATUS_STOP 0
#define STATUS_PLAY 1


namespace Ui {
class PlayBackUi;
}


class PlayBackUi : public QWidget
{
    Q_OBJECT

public:
    explicit PlayBackUi(QWidget *parent = nullptr);
    ~PlayBackUi();

private slots:
    void on_pushButton_clicked();

    void on_pushButtonPlay_clicked();

    void on_pushButtonStop_clicked();

    void on_pushButtonPre_clicked();

    void on_pushButtonNext_clicked();

    void on_pushButton_3_clicked();

    void on_pushButton_4_clicked();


    void changeNvsSwitch(const int flag);
    void onSliderPressed();

    void on_btnGetVideoData_clicked();

protected slots:
    void closeEvent(QCloseEvent *event);
    void showVideo(QByteArray data, int curCount, int playStatus);

private :
    void changePlayBtnState(const int state);
    void getFrame(const int mode);
    void updateNvsResultText(QByteArray data);

private:
    Ui::PlayBackUi *ui;

    int m_frameCount = 0;
    int m_curCountIdx = 0;
    QString m_sFilePath;
    QFile m_file;
    qint64 m_fileSize = 0;

    VideoData_S m_frameInfo;
    bool m_bPlay = false;
    NvsHandler m_nvsHandler;
    std::thread* m_thread = nullptr;
    bool m_isRunning = false;

    YuvPlay m_player;
};

#endif // PLAYBACKUI_H

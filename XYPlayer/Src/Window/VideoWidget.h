#ifndef VIDEOWIDGET_H
#define VIDEOWIDGET_H

#include <QFrame>
#include <QFutureWatcher>
#include <QElapsedTimer>
#include <QThread>

#include "Media.h"
#include "YStruct.h"
#include "VideoPlayerFactory.h"

QT_FORWARD_DECLARE_CLASS(VideoTitleBar)
QT_FORWARD_DECLARE_CLASS(VideoToolBar)
QT_FORWARD_DECLARE_CLASS(VideoWindow)
QT_FORWARD_DECLARE_CLASS(QPushButton)
QT_FORWARD_DECLARE_CLASS(VideoResMonitor)
QT_FORWARD_DECLARE_CLASS(CommWidget)
QT_FORWARD_DECLARE_CLASS(GLWindow)

class VideoWidget :  public QFrame
{
    Q_OBJECT
public:
    explicit VideoWidget(QWidget *parent = nullptr);
    ~VideoWidget();
    enum Status {
        STOP,
        PAUSE,
        PLAY,
    };


    Media getMedia() const;
    VideoPlayer *getPImpl_player() const;

    bool writeConf();

    bool checkVideoRes();
signals:
    void sigSendMsg(QString &msg);
public slots:
    void open(Media &media);
    void close();

    void start();
    void stop();
    void pause();
    void resume();
    void restart();
    void retry();

    void onTimerUpdate();
    void onOpenSucceed();
    void onOpenFailed();
    void onPlayerError();
    
    void setAspectRatio(aspect_ratio_t ar);

    bool I2CTest();
    void onUpdate();

    void onUpdateWatermark(QList<CustomWatermarkInfo> &list);
protected:
    void initUI();
    void initConnect();
    void updateUI();

    virtual void resizeEvent(QResizeEvent *e) override;
    virtual void enterEvent(QEvent *e)  override;
    virtual void leaveEvent(QEvent *e)  override;
    virtual void mousePressEvent(QMouseEvent *e)  override;
    virtual void mouseReleaseEvent(QMouseEvent *e)  override;
    virtual void mouseMoveEvent(QMouseEvent *e)  override;
    virtual void customEvent(QEvent* e)  override;


public:
    int             playerid{0};
    int             status{STOP};
    QString         title{""};
    int             fps{60};

    aspect_ratio_t  aspect_ratio;
    renderer_type_e renderer_type;

    VideoTitleBar*  titlebar{nullptr};
    VideoToolBar*   toolbar{nullptr};
    GLWindow*       videownd{nullptr};
    QPushButton*    btnMedia{nullptr};

    QThread*         thread_player{nullptr};

    Media           media;
private:
    QPoint          m_ptMousePress;
    QTimer*         timer{nullptr};

    VideoPlayer*    pImpl_player{nullptr};

    // for retry when SIGNAL_END_OF_FILE
    int             retry_interval{0};
    int             retry_maxcnt{0};
    int64_t         last_retry_time{0};
    int             retry_cnt{0};

    QFutureWatcher<bool> watcher;
    QFutureWatcher<bool> I2CWatcher;

    VideoResMonitor*    videoMonitor{nullptr};
    CommWidget*         mCommWidgt{nullptr};
};


class VideoResMonitor : public QThread{
    Q_OBJECT
public:
    VideoResMonitor(GLWindow* wnd, VideoPlayer* player) : videownd(wnd),videoplayer(player){

    }
    bool runFlag{true};
signals:
    void sigVideoResChanged();
    void sigParamLine(QByteArray param);
private:
    GLWindow*       videownd{nullptr};
    VideoPlayer*    videoplayer{nullptr};
protected:
    virtual void run() override;
};
#endif // VIDEOWIDGET_H

#include "VideoWidget.h"
#include <string.h>
#include <QVBoxLayout>

#include "VideoTitleBar.h"
#include "VideoToolBar.h"
#include "YFunction.h"
#include "styles.h"
#include "CustomEvent.h"
#include "yTime.h"
#include "YDefine.h"
#include "confile.h"
#include "VideoPlayer.h"

#include "OpenMediaDlg.h"
#include "AlgSdkManager.h"
#include "IICAddr.h"
#include "CommWidget.h"
#include "GLWindow.h"
#include "ToolsWidget.h"
#include "WatermarkTool.h"
#include "RealTimeInfoWidget.h"

static int player_event_callback(player_event_e e, void* userdata) {
    VideoWidget* wdg = (VideoWidget*)userdata;
    int custom_event_type = CustomEvent::User;
    switch (e) {
    case PLAYER_OPENED:
        custom_event_type = CustomEvent::OpenMediaSucceed;
        break;
    case PLAYER_OPEN_FAILED:
        custom_event_type = CustomEvent::OpenMediaFailed;
        break;
    case PLAYER_ERROR:
        custom_event_type = CustomEvent::PlayerError;
        break;
    default:
        return 0;
    }
    qInfo("postEvent %d", custom_event_type);
    QApplication::postEvent(wdg, new QEvent((QEvent::Type)custom_event_type));
    return 0;
}

static renderer_type_e renderer_type_enum(const std::string str) {
    if ((str == "opengl") ) {
        return RENDERER_TYPE_OPENGL;
    }
    else if (str == "qpainter") {
        return RENDERER_TYPE_QPAINTER;
    }
    return DEFAULT_RENDERER_TYPE;
}

VideoWidget::VideoWidget(QWidget *parent)
    : QFrame(parent)
{
    fps = g_config->get<int>("fps", "video", 60);
    // aspect_ratio
    std::string str = g_config->get<std::string>("aspect_ratio", "video", "w:h");
    aspect_ratio.w = 640;
    aspect_ratio.h = 512;
    const char* c_str = str.c_str();    
    aspect_ratio.type = ASPECT_FULL;
    if (str.empty() || str == "100%") {
        aspect_ratio.type = ASPECT_FULL;
    }
    else if (str == "w:h") {
        aspect_ratio.type = ASPECT_ORIGINAL_RATIO;
    }
    else if (str == "wxh" || str == "w*h") {
        aspect_ratio.type = ASPECT_ORIGINAL_SIZE;
    }
    else if (strchr(str.c_str(), '%')) {
        int percent = 0;
        sscanf(c_str, "%d%%", &percent);
        if (percent) {
            aspect_ratio.type = ASPECT_PERCENT;
            aspect_ratio.w = percent;
            aspect_ratio.h = percent;
        }
    }
    else if (strchr(c_str, ':')) {
        int w = 0;
        int h = 0;
        sscanf(c_str, "%d:%d", &w, &h);
        if (w && h) {
            aspect_ratio.type = ASPECT_CUSTOM_RATIO;
            aspect_ratio.w = w;
            aspect_ratio.h = h;
        }
    }
    else if (strchr(c_str, 'x')) {
        int w = 0;
        int h = 0;
        sscanf(c_str, "%dx%d", &w, &h);
        if (w && h) {
            aspect_ratio.type = ASPECT_CUSTOM_SIZE;
            aspect_ratio.w = w;
            aspect_ratio.h = h;
        }
    }
    else if (strchr(c_str, 'X')) {
        int w = 0;
        int h = 0;
        sscanf(c_str, "%dX%d", &w, &h);
        if (w && h) {
            aspect_ratio.type = ASPECT_CUSTOM_SIZE;
            aspect_ratio.w = w;
            aspect_ratio.h = h;
        }
    }
    else if (strchr(c_str, '*')) {
        int w = 0;
        int h = 0;
        sscanf(c_str, "%d*%d", &w, &h);
        if (w && h) {
            aspect_ratio.type = ASPECT_CUSTOM_SIZE;
            aspect_ratio.w = w;
            aspect_ratio.h = h;
        }
    }
    else {
        aspect_ratio.type = ASPECT_FULL;
    }
    // qInfo("aspect_ratio type=%d w=%d h=%d", aspect_ratio.type, aspect_ratio.w, aspect_ratio.h);

    // retry
    retry_interval = g_config->get<int>("retry_interval", "video", DEFAULT_RETRY_INTERVAL);
    retry_maxcnt = g_config->get<int>("retry_maxcnt", "video", DEFAULT_RETRY_MAXCNT);
    last_retry_time = 0;
    retry_cnt = 0;

    initUI();
    initConnect();
}

VideoWidget::~VideoWidget()
{
    close();
}
#include <QtConcurrent/QtConcurrent>
#include <QFuture>
#include <sys/types.h>
void VideoWidget::open(Media &media)
{
    qDebug() << "open:" << gettid();
    this->media = media;
    start();

    QFuture<bool> future = QtConcurrent::run(this, &VideoWidget::writeConf);

    QObject::connect(&watcher, &QFutureWatcher<bool>::finished, [=]() {

        if(!watcher.result()){
            onOpenFailed();
        }else{

            yprint(QString("ID:%1 配置文件下发成功").arg(playerid), PRINT_GREEN);
        }
    });

    // 设置 watcher 来监控 future
    watcher.setFuture(future);


    // bool ret = con.writeCameraConf(QString::fromStdString(media.src), media.group(), media.channel);
    // // bool ret = AlgSdkManager::instance()->writeCameraConf(QString::fromStdString(media.src), media.group(), media.channel);
    // if(!ret)    onOpenFailed();
}

void VideoWidget::close()
{
    stop();
    media.type = MEDIA_TYPE_NONE;
    title = "";
    updateUI();
}

void VideoWidget::start()
{
    if (media.type == MEDIA_TYPE_NONE) {
        QMessageBox::information(this, "提示", "请先设置媒体资源,再点击播放.");
        updateUI();
        return;
    }
    if (!pImpl_player) {
        pImpl_player = VideoPlayerFactory::create(media.type);

        pImpl_player->set_media(media);
        pImpl_player->set_event_callback(player_event_callback, this);
        title = media.src.c_str();
        if(videoMonitor) {
            delete videoMonitor;
            videoMonitor = nullptr;
        }

        videoMonitor  = new VideoResMonitor(videownd, pImpl_player);
        videoMonitor->runFlag = true;
        connect(videoMonitor, &VideoResMonitor::sigVideoResChanged, this, &VideoWidget::onUpdate);
        connect(videoMonitor, &VideoResMonitor::sigParamLine, mCommWidgt->realTimeInfoWidget, &RealTimeInfoWidget::setParamLine);
        videoMonitor->start();

        bool ret = pImpl_player->start();
        if (ret != 0) {
            onOpenFailed();
        }
        else {
            onOpenSucceed();
        }
        updateUI();
    }
    else {
        if (status == PAUSE) {
            resume();
        }
    }
}

void VideoWidget::stop()
{
    // timer->stop();
    qDebug() << "start delete player";
    if (pImpl_player) {
        videoMonitor->runFlag = false;



        pImpl_player->stop();
        QThread::msleep(1000);
        delete pImpl_player;
        pImpl_player = nullptr;
    }
    qDebug() << "end delete player";
    if(videownd && !videownd->frame.isNull())
        videownd->frame.clear();

    status = STOP;

    last_retry_time = 0;
    retry_cnt = 0;
    if(videownd)
        videownd->update();
    qDebug() << "videownd->update";
    updateUI();
}

void VideoWidget::pause()
{
    if (pImpl_player) {
        pImpl_player->pause();
    }
    // timer->stop();
    status = PAUSE;

    updateUI();
}

void VideoWidget::resume()
{
    if (status == PAUSE && pImpl_player) {
        pImpl_player->resume();
        // timer->start(1000 / fps );
        status = PLAY;

        updateUI();
    }
}

void VideoWidget::restart()
{
    qInfo("restart...");
    if (pImpl_player) {
        pImpl_player->stop();
        pImpl_player->start();
    }
    else {
        start();
    }
}

void VideoWidget::retry()
{
    if (retry_maxcnt < 0 || retry_cnt < retry_maxcnt) {
        ++retry_cnt;
        int64_t cur_time = CurMSecsSinceEpoch();
        int64_t timespan = cur_time - last_retry_time;
        if (timespan >= retry_interval) {
            last_retry_time = cur_time;
            restart();
        }
        else {
            last_retry_time += retry_interval;
            if (pImpl_player) {
                pImpl_player->stop();
            }
            int retry_after = retry_interval - timespan;
            qInfo("retry after %dms", retry_after);
            QTimer::singleShot(retry_after, this, SLOT(restart()));
        }
    }
    else {
        stop();
    }
}

void VideoWidget::onTimerUpdate()
{
    if (pImpl_player == NULL)   return;
    // qint64 elapsed = elapsedTimer.elapsed();
    //     if (elapsed < 16) {
    //         QThread::msleep(16 - elapsed);
    //     }
    //     elapsedTimer.restart();
    // qDebug() << QString("update video id:%1").arg(playerid);

    // if(playerid == 1)
        qDebug() << "Id:" << playerid << "time interval:" << QDateTime::currentDateTime().toString("hh:mm:ss zzz") << " set interval:" << 1000 / fps;;
    // if(pImpl_player->get_frame_size() > 50){
    //     timer->stop();
    //     fps = 60;
    //     timer->start(1000/60);
    //     yprint(QString("Warning: Id:%3 size:%1,restart timer[fps:%2]").arg(pImpl_player->get_frame_size()).arg(fps).arg(playerid));
    // }
    if (pImpl_player->pop_frame(videownd->last_frame)) {

        // if(pImpl_player->get_frame_size() > 5){
        //     if(fps != 60){
        //         fps = 60;//缓存压力较大时，加大帧率，缓解缓存压力
        //         timer->setInterval(1000 / fps);
        //         qDebug() << "Play at 60fps" << " bufSize:" << pImpl_player->get_frame_size();
        //     }
        // }else{
        if(pImpl_player->fps > 0 && pImpl_player->fps <= 30) {
            if(fps != pImpl_player->fps+5){
                fps = pImpl_player->fps+5;
                g_config->set<int>("fps", fps, "video");
                auto intervals = 1000 / fps;
                // timer->setInterval(intervals);
                qDebug() << QString("Play at %1 intervals").arg(fps) ;
            }
        }
        // }

        // update progress bar
        // if (toolbar->sldProgress->isVisible()) {
        //     int progress = (videownd->last_frame.ts - pImpl_player->start_time) / 1000;
        //     if (toolbar->sldProgress->value() != progress &&
        //         !toolbar->sldProgress->isSliderDown()) {
        //         toolbar->sldProgress->setValue(progress);
        //     }
        // }
        // update video frame
        if(videownd->last_frame){
            videownd->update();
        }

    }
    // else{
    //     qDebug() << "Unable to retrieve data. please check the queue. curTime:" << QDateTime::currentDateTime().toString("hh:mm:ss zzz");
    // }
}

void VideoWidget::onOpenSucceed()
{
    // timer->start(1000 / fps);
    // if(!timer_update){
        // timer_update = new HighResTimer(16, [this](){
        //     onTimerUpdate(); elapsedTimer.start();
        // });
    // }

    status = PLAY;
    setAspectRatio(aspect_ratio);
    if (pImpl_player->duration > 0) {
        int duration_sec = pImpl_player->duration / 1000;
        auto szTime = duration_fmt_hms(duration_sec);
        toolbar->lblDuration->setText(szTime);
        toolbar->sldProgress->setRange(0, duration_sec);
        toolbar->lblDuration->show();
        toolbar->sldProgress->show();
    }

    if (retry_cnt != 0) {
        qInfo("retry succeed: cnt=%d media.src=%s", retry_cnt, media.src.c_str());
    }
}

void VideoWidget::onOpenFailed()
{
    if (retry_cnt == 0) {
        QString msg = QString("无法打开媒体: \n")+QString::fromStdString(media.src)+
                QString::asprintf("\nerrcode=%d", pImpl_player->error);
        QMessageBox::critical(this, "错误", msg);
        stop();
    }
    else {
        qInfo("retry failed: cnt=%d media.src=%s", retry_cnt, media.src.c_str());
        retry();
    }
}


void VideoWidget::onPlayerError()
{
    switch (media.type) {
    case MEDIA_TYPE_NETWORK:
        retry();
        break;
    default:
        stop();
        break;
    }
}

void VideoWidget::setAspectRatio(aspect_ratio_t ar)
{
   aspect_ratio = ar;
   int border = 1;
   int scr_w = width() - border * 2;
   int scr_h = height() - border * 2;
   if (scr_w <= 0 || scr_h <= 0) return;
   int pic_w = 0;
   int pic_h = 0;
   if (pImpl_player && status != STOP) {
       pic_w = pImpl_player->width;
       pic_h = pImpl_player->height;
   }
   if (pic_w == 0) pic_w = scr_w;
   if (pic_h == 0) pic_h = scr_h;
   // calc videownd rect
   int dst_w, dst_h;
   switch (ar.type) {
   case ASPECT_FULL:
       dst_w = scr_w;
       dst_h = scr_h;
       break;
   case ASPECT_PERCENT:
       dst_w = pic_w * ar.w / 100;
       dst_h = pic_h * ar.h / 100;
       break;
   case ASPECT_ORIGINAL_SIZE:
       dst_w = pic_w;
       dst_h = pic_h;
       break;
   case ASPECT_CUSTOM_SIZE:
       dst_w = ar.w;
       dst_h = ar.h;
       break;
   case ASPECT_ORIGINAL_RATIO:
   case ASPECT_CUSTOM_RATIO:
   {
       double scr_ratio = (double)scr_w / (double)scr_h;
       double dst_ratio = 1.0;
       if (ar.type == ASPECT_CUSTOM_RATIO) {
           dst_ratio = (double)ar.w / (double)ar.h;
       }
       else {
           dst_ratio = (double)pic_w / (double)pic_h;
       }
       if (dst_ratio > scr_ratio) {
           dst_w = scr_w;
           dst_h = scr_w / dst_ratio;
       }
       else {
           dst_h = scr_h;
           dst_w = scr_h * dst_ratio;
       }
   }
   break;
   }
   dst_w = std::min(dst_w, scr_w);
   dst_h = std::min(dst_h, scr_h);
   // align 4
   dst_w = dst_w >> 2 << 2;
   dst_h = dst_h >> 2 << 2;

   int x = border + (scr_w - dst_w) / 2;
   int y = border + (scr_h - dst_h) / 2;
   videownd->setGeometry(QRect(x, y, dst_w, dst_h));
}

void VideoWidget::initUI()
{
    setFocusPolicy(Qt::ClickFocus);

    mCommWidgt = new CommWidget(playerid);
    videownd = new GLWindow(this);
    titlebar = new VideoTitleBar(this);
    toolbar  = new VideoToolBar(this);
    btnMedia = genPushButton(QPixmap(":XYPlayer"), "打开媒体");

    QVBoxLayout *vbox = genVBoxLayout();

    vbox->addWidget(titlebar, 0, Qt::AlignTop);
    vbox->addWidget(btnMedia, 0, Qt::AlignCenter);
    vbox->addWidget(toolbar,  0, Qt::AlignBottom);

    setLayout(vbox);

    titlebar->hide();
    toolbar->hide();
}

bool VideoWidget::I2CTest()
{
    IICControl IICCtrl;
    bool ret  = IICCtrl.writeRegByIIC(media.channel,  SYSTEM_CONTROL_SHUTTER_SWITCH, 0);
    readReg_S read_s = IICCtrl.readRegByIIC(media.channel, SYSTEM_CONTROL_AUTO_COMP_TIME);


    ret = IICCtrl.writeRegByIIC(media.channel, SYSTEM_CONTROL_ALG_TYPE, 7);
    QThread::msleep(500);
    if(!ret) return false;
    ret = IICCtrl.writeRegByIIC(media.channel, SYSTEM_CONTROL_SR_UNLOCK, 0x8012); //最高位：0:写 1:读 低8位：寄存器地址0x12 算法参数地址
    QThread::msleep(500);
    read_s = IICCtrl.readRegByIIC(media.channel, SYSTEM_CONTROL_SR_UNLOCK);
    if(!ret) return false;

    qDebug() << QDateTime::currentDateTime().toString("hh:mm:ss zzz");
    QThread::msleep(1000);
    qDebug() << QDateTime::currentDateTime().toString("hh:mm:ss zzz");

    read_s = IICCtrl.readRegByIIC(media.channel, SPECIAL_CONFIG_ISP_ALG, 58);
    // if(read_s.len > 0){
    //     for(auto i = 0;i < read_s.len;++i){
    //         qDebug("Recv:%d",read_s.value[i]);
    //     }
    // }
    ret = IICCtrl.writeRegByIIC(media.channel, SYSTEM_CONTROL_SR_READ_END, 1);//读完成 回写1
    if(!ret) return false;

    QThread::msleep(500);
    ret  = IICCtrl.writeRegByIIC(media.channel,  IMAGE_SETTING_SET_COLOR, 22);


    read_s = IICCtrl.readRegByIIC(media.channel, IMAGE_SETTING_SET_COLOR);
    return read_s.ret;


    // return AlgSdkManager::instance()->writeReg(0, media.channel, 0x66, SYSTEM_CONTROL_SHUTTER_SWITCH, 0, 0x1616);
}

void VideoWidget::initConnect()
{
    connect( btnMedia, &QPushButton::clicked, [this] {
        OpenMediaDlg dlg(playerid, this);
        if (dlg.exec() == QDialog::Accepted)  open(dlg.media);   
    });
    
    connect( titlebar->m_btnClose, &QPushButton::clicked, this, &VideoWidget::close);

    connect( toolbar, &VideoToolBar::sigStart, this, &VideoWidget::start );
    connect( toolbar, &VideoToolBar::sigPause, this, &VideoWidget::pause );
    connect( toolbar, &VideoToolBar::sigStop,  this, &VideoWidget::stop  );
    connect( toolbar->sldProgress, &QSlider::sliderReleased, [this]() {
        if (pImpl_player)  pImpl_player->seek(toolbar->sldProgress->value()*1000);
    });
    connect(toolbar->m_btnRecord, &QPushButton::clicked, this, [=]{
        if(!pImpl_player || status != PLAY){
            QString msg = QString("窗口%1 当前无法录制视频!").arg(playerid);
            emit sigSendMsg(msg);
            return;
        }
        if(!pImpl_player->isRecord()){
            pImpl_player->startRecord();
            if(videownd) videownd->setRecord(true);
        } else {
            pImpl_player->stopRecord();
            if(videownd) videownd->setRecord(false);
        };
            
    });

    connect( toolbar->m_btnScreenShot, &QPushButton::clicked, this, [=]{
        if(!pImpl_player || status != PLAY){
            QString msg = QString("窗口%1 当前无法截图!").arg(playerid);
            emit sigSendMsg(msg);
            return;
        }
        pImpl_player->screenShot();
        QString msg = QString("窗口%1 截图成功!").arg(playerid);
        emit sigSendMsg(msg);
    });

    connect( toolbar->m_btnFunWind, &QPushButton::clicked, this, [=]{
        qDebug() << media.toString();
        mCommWidgt->setMedia(media);
        mCommWidgt->show();
        // IICControl IICCtrl;
        // // IICCtrl.writeRegByIIC(media.channel,  SYSTEM_CONTROL_SHUTTER_SWITCH, 1);
        // QFuture<bool> future = QtConcurrent::run(this, &VideoWidget::I2CTest);
        // QObject::connect(&I2CWatcher, &QFutureWatcher<bool>::finished, [=]() {
        //     if(!I2CWatcher.result()){
        //         yprint(QString("ID:%1 IIC 通信失败").arg(playerid));
        //     }else{
        //         yprint(QString("ID:%1 IIC 通信成功").arg(playerid), PRINT_GREEN);
        //     }
        // });
        // // 设置 watcher 来监控 future
        // I2CWatcher.setFuture(future);

    });

    connect(mCommWidgt->toolsWidget->watermarkTool, &WatermarkTool::sigWatermark, this, &VideoWidget::onUpdateWatermark);


    // timer = new QTimer(this);
    // timer->setTimerType(Qt::PreciseTimer);
    // connect(timer, &QTimer::timeout, this, &VideoWidget::onTimerUpdate);

}

void VideoWidget::updateUI()
{
    titlebar->m_labTitle->setText(QString::asprintf("窗口%d-通道%d", playerid, media.channel+1));

    toolbar->m_btnStart->setVisible(status != PLAY);
    toolbar->m_btnPause->setVisible(status == PLAY);

    btnMedia->setVisible(status == STOP);

    if (status == STOP) {
        toolbar->sldProgress->hide();
        toolbar->lblDuration->hide();
    }
}

void VideoWidget::resizeEvent(QResizeEvent *e)
{
    Q_UNUSED(e)
    setAspectRatio(aspect_ratio);
}

void VideoWidget::enterEvent(QEvent *e)
{
    Q_UNUSED(e)
    updateUI();

    titlebar->show();
    toolbar->show();
}

void VideoWidget::leaveEvent(QEvent *e)
{
    Q_UNUSED(e)
    titlebar->hide();
    toolbar->hide();
}

void VideoWidget::mousePressEvent(QMouseEvent *e)
{
    m_ptMousePress = e->pos();
#if WITH_MV_STYLE
    e->ignore();
#endif
}

void VideoWidget::mouseReleaseEvent(QMouseEvent *e)
{
#if WITH_MV_STYLE
    e->ignore();
#endif
}

void VideoWidget::mouseMoveEvent(QMouseEvent *e)
{
#if WITH_MV_STYLE
    e->ignore();
#endif
}

void VideoWidget::customEvent(QEvent *e)
{
    switch(e->type()) {
    case CustomEvent::OpenMediaSucceed:
        onOpenSucceed();
        break;
    case CustomEvent::OpenMediaFailed:
        onOpenFailed();
        break;
    case CustomEvent::PlayerError:
        onPlayerError();
        break;
    default:
        break;
    }
}

Media VideoWidget::getMedia() const
{
    return media;
}

VideoPlayer *VideoWidget::getPImpl_player() const
{
    return pImpl_player;
}

bool VideoWidget::writeConf()
{
    ConsoleControl con;
    return con.writeCameraConf(QString::fromStdString(media.src), media.group(), media.channel);
}

void VideoWidget::onUpdate()
{
    videownd->update();
}

void VideoWidget::onUpdateWatermark(QList<CustomWatermarkInfo> &list)
{
    videownd->watermarkList = list;
}

void VideoResMonitor::run()
{
    int cnt{0};
    while(runFlag){
        if(videoplayer->pop_frame(videownd->last_frame)){
            videownd->frame.copy(*videownd->last_frame.get());
            emit sigVideoResChanged();
            cnt++;
            if(cnt >= 10){
                cnt = 0;
                QByteArray array = QByteArray::fromRawData((char*)videownd->last_frame->param.data(),videownd->last_frame->param.size());
                emit sigParamLine(array);
            }
        }
    }
}

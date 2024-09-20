#include "VideoWindow.h"
#include "confile.h"
#include <QTime>
#include <QDebug>

VideoWindow::VideoWindow()
{
    fps = 0;
    framecnt = 0;
    tick = 0;
    draw_time = g_config->get<bool>("draw_time", "ui", false);
    draw_fps  = g_config->get<bool>("draw_fps", "ui", false);
    draw_resolution = g_config->get<bool>("draw_resolution", "ui", false);
    recordTimer = new QTimer(this);
    connect(recordTimer, &QTimer::timeout, this, [=]{
        time = time.addSecs(1);
    });
}

void VideoWindow::setRecord(bool state)
{
    draw_record = state;
    if(draw_record == true){
        if(recordTimer->isActive()){
            recordTimer->stop();
        }
        time = QTime(0, 0, 0);
        recordTimer->start(1000);
    }else{
        recordTimer->stop();
    }
}


void VideoWindow::calcFPS()
{
//    if (gettick() - tick > 1000) {
//        fps = framecnt;
//        framecnt = 0;
//        tick = gettick();
//    }
//    else {
//        ++framecnt;
//    }
}

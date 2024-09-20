#ifndef YTIME_H
#define YTIME_H
#include <iostream>
#include <QString>
#include <QTime>
#include <QDateTime>
#define TIME_FMT            "%02d:%02d:%02d"

char* duration_fmt(int sec, char* buf) {
    int h, m, s;
    m = sec / 60;
    s = sec % 60;
    h = m / 60;
    m = m % 60;
    sprintf(buf, TIME_FMT, h, m, s);
    return buf;
}

QString duration_fmt_hms(int sec){
    return QTime(0,0,0).addSecs(sec).toString("hh:mm:ss");
}

qint64 CurMSecsSinceEpoch(){
    return QDateTime::currentDateTime().toMSecsSinceEpoch();
}
#endif // YTIME_H

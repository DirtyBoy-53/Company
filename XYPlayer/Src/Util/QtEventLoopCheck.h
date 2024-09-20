#ifndef QT_EVENTLOOP_CHECK_H
#define QT_EVENTLOOP_CHECK_H
#include <QApplication>
#include <QObject>
#include <QTimer>
#include <QElapsedTimer>
#include <QDebug>
#include <YFunction.h>

class EventLoopCheck : public QObject
{
    Q_OBJECT
public:
    EventLoopCheck(QObject *parent=nullptr) : QObject(parent) {
        timer = new QTimer(this);
        connect(timer, &QTimer::timeout, this, &EventLoopCheck::checkEventLoop);
        timer->start(1000); // 每秒检查一次
        lastCheckTime.start();
    }

private slots:
    void checkEventLoop() {
        qint64 elapsed = lastCheckTime.elapsed();
        if (elapsed > 1050) { // 如果超过预期的 1000 毫秒（加上一个小余量5ms）
            QString msg =  QString("Event loop is lagging! Elapsed time:%1ms").arg(elapsed);
            yprint(msg, PRINT_GRAY);
        }
        lastCheckTime.restart();
    }

private:
    QTimer *timer;
    QElapsedTimer lastCheckTime;
};

#endif

#include <iostream>
#include <QApplication>
#include "mainwindow.h"
#include "alg_sdk/alg_sdk.h"
#include <thread>
#include <QThread>
#include "alg_sdk/service.h"
#include <QString>
#include <QProcess>
#include <stdlib.h>
#include <QFileInfo>
#define PTPSERVER_SHELL_NAME    "startPTPServer.sh"
#define APP_PATH  (qApp->applicationDirPath())
#define SKIN_STYLE  "lightstyle"    //lightstyle darkstyle
inline void loadSkin(const char *skin) {
    // :/skin/dark.qss
    // QFile file(QString(":/skin/") + QString(skin) + QString(".qss"));
    QFile file(QString(":%1").arg(skin));
    if (file.open(QFile::ReadOnly)) {
        qApp->setStyleSheet(file.readAll());
        file.close();
    }else{
        // Log::Logger->warn("read {}.qss error.",skin);
    }
}

void startDemo()
{
    int rc;
    char *appsrc[] = {"--publish"};
    rc = alg_sdk_init_v2(1, &appsrc[0]);
    if(rc < 0)
    {
        qInfo("Init SDK Failed\n");
//        exit(0);
     }

     alg_sdk_spin_on();
     qInfo("start sdk succ");
}

void start_shell(){
    QString ptp_shell_path = APP_PATH + "/base_Script/" + PTPSERVER_SHELL_NAME;
    QFileInfo shell_info(ptp_shell_path);
    if(shell_info.isFile()){
        QString excute_cmd = QString("gnome-terminal --tab -- bash -c '%1;exec bash'").arg(ptp_shell_path);
        auto rt = system(excute_cmd.toStdString().c_str());
        qDebug() << "Info:" + ptp_shell_path;
    }else{
        qDebug() << "Error:can`t find " + ptp_shell_path;
    }
}

int main(int argc,char *argv[])
{
    QApplication app(argc,argv);
    start_shell();
    
    new std::thread(startDemo);
    QThread::sleep(5);
    
    // loadSkin(SKIN_STYLE);

    MainWindow w;
    w.show();

    return app.exec();
}
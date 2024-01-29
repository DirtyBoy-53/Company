#include "qtheaders.h"
#include "qtstyles.h"
#include "appdef.h"
#include "confile.h"
//#include <format>
#include "mainwindow.h"
#include <iostream>

IniParser* g_config = nullptr;
char g_exec_path[256]{0};
char g_exec_dir[256]{0};
char g_run_dir[256]{0};
char g_conf_file[256]{0};
char g_log_file[256]{0};


static void qLogHandler(QtMsgType type, const QMessageLogContext & ctx, const QString & msg) {

    //enum QtMsgType { QtDebugMsg, QtWarningMsg, QtCriticalMsg, QtFatalMsg, QtInfoMsg, QtSystemMsg = QtCriticalMsg };
    static char s_types[5][6] = {"DEBUG", "WARN ", "ERROR", "FATAL", "INFO "};
    const char* szType = "DEBUG";
    if (type < 5) {
        szType = s_types[(int)type];
    }

#ifdef QT_NO_DEBUG
    QString strLog = QString::asprintf("[%s][%s] %s\n",
                                       QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss.zzz").toLocal8Bit().data(),
                                       szType,
                                       msg.toLocal8Bit().data());
    QString logpath = QString("%1/logs/").arg(g_exec_dir);
    QDir dir(logpath);
    if(dir.exists() == false){
        dir.mkpath(logpath);
    }
    logpath.append(QDateTime::currentDateTime().toString("yyyy-MM-dd")+"-qt.log");
    QFile file(logpath);
    if (file.open(QIODevice::WriteOnly | QIODevice::Append)) {
        file.write(strLog.toLocal8Bit());
        file.close();
    }
#else
    QString strLog = QString::asprintf("%d-%s:[%s] [%s]",
                                       ctx.line,ctx.function,szType,
                                       msg.toLocal8Bit().data()
                                       );
    qDebug() << strLog;
#endif

}

static int load_config(){
    get_executable_path(g_exec_path, sizeof(g_exec_path));
#if 1
    get_executable_dir(g_exec_dir,sizeof(g_exec_dir));
    get_run_dir(g_run_dir,sizeof(g_run_dir));

    qInfo("g_exec_path:%s",g_exec_path);
    qInfo("g_exec_dir:%s",g_exec_dir);
    qInfo("g_run_dir:%s",g_run_dir);

    g_config = new IniParser;
    snprintf(g_conf_file,sizeof(g_conf_file), "%s/conf/%s.conf",g_exec_dir,APP_NAME);
    if(access(g_conf_file, 0) != 0){
        QFile::copy(QString(g_exec_dir) + "/conf/" APP_NAME ".conf.default", g_conf_file);
    }
    g_config->LoadFromFile(g_conf_file);
    // logfile
    std::string str = g_config->GetValue("logfile");
    if(str.empty()){
//        std::format_to(std::back_inserter(str),"logs/.log",APP_NAME);
        str.append("logs/").append(APP_NAME).append(".log");
    }
    snprintf(g_log_file, sizeof(g_log_file), "%s/%s",g_exec_dir, str.c_str());
    YLog::initLog(g_log_file);

    // first log here
    YLog::Logger->info("{} version: {}",g_exec_path, hv_compile_version());
#endif
    return 1;

}

int window_init(MainWindow& window)
{
#if 1
    window.window_state = (MainWindow::window_state_e)g_config->Get<int>("main_window_state","ui");
    switch (window.window_state)
    {
    case MainWindow::FULLSCREEN :
        window.showFullScreen(); break;
    case MainWindow::MAXIMIZED :
        window.showMaximized(); break;
    case MainWindow::MINIMIZED :
        qDebug() << "showMinimized enter";
        window.showMinimized(); break;
        qDebug() << "showMinimized end";
    default:
        std::string str = g_config->GetValue("main_window_rect","ui");

        if(!str.empty()){
            int x,y,w,h;
            sscanf(str.c_str(),"rect(%d,%d,%d,%d)",&x,&y,&w,&h);
            if(w&h){
                window.setGeometry(x,y,w,h);
            }
        }
        window.show();
        break;
    }
    if(g_config->Get<bool>("mv_fullscreen","ui")){
//        window.mv_fullscreen();
    }
#endif

    return 1;
}


int main(int argc,char** argv)
{
    load_config();
    qInstallMessageHandler(qLogHandler);
    qInfo("================<app start>================");
    QApplication a(argc,argv);
    a.setApplicationName(APP_NAME);

    std::string str = g_config->GetValue("skin","ui");
    loadSkin(str.empty() ? DEFAULT_SKIN : str.c_str());

    str = g_config->GetValue("palette","ui");
    setPalette(str.empty() ? DEFAULT_PALETTE_COLOR : strtoul(str.c_str(),NULL,16));

    setFont(g_config->Get<int>("fontsize","ui",DEFAULT_FONT_SIZE));
    int exitcode{0};
    MainWindow *w{nullptr};
    try{
        w = new MainWindow;
        window_init(*w);

        exitcode = a.exec();
    } catch (const char *errorStr){
        qDebug()<<errorStr;
        QString msg;
        msg = QString("The program has crashed.\n")
              +"Message: "+errorStr;
        QMessageBox::warning(nullptr, "Program crashed", msg);
    }

    qInfo("================<app end>================");
    g_config->Set<int>("main_window_state", (int)w->window_state,"ui");
    str = asprintf("rect(%d,%d,%d,%d)",w->x(),w->y(),w->width(),w->height());
    g_config->SetValue("main_window_rect",str,"ui");
    g_config->Save();
    SAFE_DELETE(g_config);
    return exitcode;

}

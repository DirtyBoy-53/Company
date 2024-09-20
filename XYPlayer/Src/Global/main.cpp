#include "appdef.h"
#include "confile.h"
#include "Window.h"
#include "ylog.h"
#include "styles.h"
#include "YFunction.h"
#include "CYamlParser.h"
#include "FormLoading.h"
#include "AlgSdkManager.h"

#include <QFileInfo>
#include <QApplication>
#include <iostream>
#include <QMessageBox>

std::unique_ptr<CYamlParser> g_config{nullptr};
QString g_exec_file_path;
QString g_exec_dir_path;

QString g_log_path;
QString g_conf_path;
static void qLogHandler(QtMsgType type, const QMessageLogContext & ctx, const QString & msg) {

    //enum QtMsgType { QtDebugMsg, QtWarningMsg, QtCriticalMsg, QtFatalMsg, QtInfoMsg, QtSystemMsg = QtCriticalMsg };
    static char s_types[5][6] = {"DEBUG", "WARN ", "ERROR", "FATAL", "INFO "};
    const char* szType = "DEBUG";
    if (type < 5) {
        szType = s_types[(int)type];
    }

#ifdef QT_NO_DEBUG
    switch (type) {
        case QtDebugMsg: YLog::Logger->debug(msg.toStdString()); break;
        case QtWarningMsg: YLog::Logger->warn(msg.toStdString()); break;
        case QtCriticalMsg: YLog::Logger->critical(msg.toStdString()); break;
        case QtFatalMsg: YLog::Logger->error(msg.toStdString()); break;
        case QtInfoMsg: YLog::Logger->info(msg.toStdString()); break;
        default: YLog::Logger->error(msg.toStdString()); break;
    }
#else
    QString strLog = QString::asprintf("%s:%d-%s:[%s] [%s]\n",
                                       ctx.file,ctx.line,ctx.function,szType,
                                       msg.toLocal8Bit().data()
                                       );

#if defined(Q_CC_MSVC)
//    OutputDebugString((LPCWSTR)strLog.toStdString().c_str());
    qDebug() << strLog;
#elif defined(Q_CC_MINGW)
    qDebug() << strLog;
#elif defined(Q_OS_UNIX)
    std::cout << strLog.toStdString();
#endif
#endif

}

static bool load_config(){

    g_exec_dir_path = QApplication::applicationDirPath();
    g_exec_file_path = QApplication::applicationFilePath();
    qInfo() << "g_exec_dir_path:" << g_exec_dir_path;
    qInfo() << "g_exec_file_path:" << g_exec_file_path;
    g_config = std::make_unique<CYamlParser>();
    g_conf_path = QString::asprintf("%s/conf/%s.yaml",g_exec_dir_path.toStdString().c_str(), APP_NAME);
    qInfo() << "g_conf_path:" << g_conf_path;

    if(!confirmFile(g_conf_path)) return false;
    g_config->loadFromFile(g_conf_path.toStdString());

    // logfile
    QString str = g_config->get<QString>("logfile","root",QString("logs/")+QString(APP_NAME) + QString(".log"));

    g_log_path = QString::asprintf("%s/%s",g_exec_dir_path.toStdString().c_str(),str.toStdString().c_str());

    YLog::initLog(g_log_path.toStdString());

    // first log here
    YLog::Logger->info("{} version: {}",g_exec_file_path.toStdString(), APP_VERSION);
    return true;
}

int window_init(Window& window)
{
    window.window_state = (Window::window_state_e)g_config->get<int>("main_window_state","ui");
    switch (window.window_state)
    {
    case Window::FULLSCREEN :
        window.showFullScreen(); break;
    case Window::MAXIMIZED :
        window.showMaximized(); break;
    case Window::MINIMIZED :
        qDebug() << "showMinimized enter";
        window.showMinimized(); break;
        qDebug() << "showMinimized end";
    default:
        std::string str = g_config->get<std::string>("main_window_rect","ui","rect(100,100,600,800)");

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
    if(g_config->get<bool>("mv_fullscreen","ui",false)){
        window.mv_fullscreen();
    }

    return 1;
}


int main(int argc,char** argv)
{

    // qInstallMessageHandler(qLogHandler);
    qInfo("================<app start>================");
    QApplication a(argc,argv);
    a.setApplicationName(APP_NAME);
    if(!load_config()) {
        qWarning() << "Failed to load the configuration file.";
        return 0;
    }

    std::string str = g_config->get<std::string>("skin","ui", DEFAULT_SKIN);
    loadSkin(str.c_str());

    str = g_config->get<std::string>("palette","ui",std::to_string(DEFAULT_PALETTE_COLOR));
    setPalette(strtoul(str.c_str(),NULL,10));

    setFont(g_config->get<int>("fontsize","ui",DEFAULT_FONT_SIZE));
    int exitcode{0};

    Window w;
    auto formload = new FormLoading(&w);
    try{
        bool exit{false};
        if(formload->exec() != QDialog::Accepted){
            QMessageBox::StandardButton reply;
            reply = QMessageBox::question(&w, "Message Box", "Are you sure to quit？", QMessageBox::Yes | QMessageBox::No);
            if(reply == QMessageBox::Yes)   exit = true;
        }
        if(!exit){
            // formload.close();
            AlgSdkManager::instance()->setWindow(&w);
            window_init(w);
            exitcode = a.exec();
        }
    } catch (const char *errorStr){
        qWarning() << errorStr;
        QString msg;
        msg = QString("The program has crashed.\n")
              +"Message: "+errorStr;
        QMessageBox::warning(nullptr, "Program crashed", msg);
    }

    qInfo("================<app end>================");
    if(formload) delete formload;
    formload = nullptr;
    g_config->set<int>("main_window_state", (int)w.window_state,"ui");
    auto rect = QString::asprintf("rect(%d,%d,%d,%d)",w.x(),w.y(),w.width(),w.height());
    g_config->set("main_window_rect",QVariant(rect),"ui");
    g_config->save();
    g_config = nullptr;
    return exitcode;

}

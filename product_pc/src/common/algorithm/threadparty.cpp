#include "threadparty.h"
#include <QLibrary>
#include <atomic>
#include <QCoreApplication>

typedef int (*Evaluate_HorStripeFun)(double *value1, double *value2, double *value3, uint16_t *pus_src, int n_width, int n_height);

static std::atomic_int g_iLoad;
static QLibrary g_lib;
static Evaluate_HorStripeFun g_HorStripeFun = nullptr;

ThreadParty::ThreadParty()
{
    g_iLoad = 0;
    if(!g_lib.isLoaded()) {
        g_lib.setFileName(QCoreApplication::applicationDirPath() + "/StriationDetection.dll");
        if(g_lib.load()) {
           g_HorStripeFun = (Evaluate_HorStripeFun)g_lib.resolve("Evaluate_HorStripe");
        }else {
            qInfo("load failed %s", g_lib.fileName().toStdString().data());
        }
    }
}

void ThreadParty::Evaluate_HorStripe(double *value1, double *value2, double *value3, uint16_t *pus_src, int n_width, int n_height)
{
    if(g_HorStripeFun) {
        g_HorStripeFun(value1,value2,value3,pus_src,n_width, n_height);
    }
}

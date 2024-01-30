#ifndef _MAINWINDOW_H_
#define _MAINWINDOW_H_

#include <QMainWindow>
#include "cameraparamwidget.h"
#include "systemparamwidget.h"
#include "cameradiswidget.h"
// #include "replaywidget.h"
#include "common.h"
#include "transconfig.h"
#include "systemsetwidget.h"
#include "eventwidget.h"
#include "playbackui.h"
#include "lidarviewerwidget.h"
#include "plotviewer.h"
#include "sensorfusion.h"
#include "matchfusionwidget.h"
namespace Ui{
    class MainWindow;
} // namespace Ui

class MainWindow : public QMainWindow{
    Q_OBJECT
public:
    explicit MainWindow(QWidget *parent=nullptr);
    ~MainWindow();

private:
    Ui::MainWindow *ui;
    cameraparamwidget *m_cameraparamwidget{nullptr};
    SystemSetWidget *m_systemSetWidget{nullptr};
    cameradiswidget *m_cameradiswidget{nullptr};
    LidarViewerWidget *m_lidarViewerWidget{nullptr};
    // replaywidget *m_replaywidget{nullptr};
    PlayBackUi *m_playBackUi{nullptr};
    TransConfig * m_transwidget{nullptr};
    EventWidget *m_eventWidget{nullptr};
    PlotViewer *m_plotViewer{nullptr};
    SensorFusion *m_sensorFusion{nullptr};
    MatchFusionWidget *m_matchFusionWidget{nullptr};
    void init();


public slots:
    void slot_bt_clicked();
};
#endif

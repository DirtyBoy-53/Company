#ifndef PLOTVIEWER_H
#define PLOTVIEWER_H

#include <QWidget>
#include <qcustomplot.h>
#include "common.h"
#include "utils/GQueue.h"
#include "radar_ars408.h"
namespace Ui {
class PlotViewer;
}
#define RADARNUM    3   //毫米波雷达数量
#define ISSHOWCONTENT   false   //是否显示点文本内容
class PlotViewer : public QWidget
{
    Q_OBJECT

public:
    explicit PlotViewer(QWidget *parent = nullptr);
    ~PlotViewer();

public slots:
    void slot_set_radar_path(QString &path);
    void slot_RadaroneFrameProc(StructRadarDataPtr data);
protected:  
    bool eventFilter(QObject *target, QEvent *event);
private:
    Ui::PlotViewer *ui;
    QCPItemText* m_sameTimeTextTip[RADARNUM][RADAR_ARS408_TABLE_MAX]{nullptr};
    QCustomPlot* m_customPlot[RADARNUM];

    bool m_dev0_is_master_radar{true};

    GQueue<StructRadarDataPtr> m_queue;
    bool m_isRun{true};
    QString m_savePath{""};
    Radar_Ars408 *m_radar_ars408[2]{nullptr};
    void init();
    int getDevIdx(int curDev,int curdev);
    void doWork();
    void dis_plot(int8_t idx, QVector<double> &_id, QVector<double> &_x, QVector<double> &_y);

    void radar_init(int port);

};

#endif // PLOTVIEWER_H

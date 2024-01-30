#pragma once
#include <QWidget>
#include <QLabel>
#include "sensormatch.h"
#include <QFuture>
#include <QFutureWatcher>
#include <QtConcurrent>
#include "datacollector.h"
#include <QTimer>
#include <QPushButton>
#include <QPixmap>
#include "imageWidget.h"
class MatchFusionWidget : public QWidget{
    Q_OBJECT
public:
    MatchFusionWidget(QWidget* parent = nullptr);
    virtual void timerEvent(QTimerEvent* event);
    virtual ~MatchFusionWidget();
protected:
    void initUI();
    void initConnect();
    bool eventFilter(QObject* watched, QEvent* event);
private:
    QPushButton* btn_start{nullptr};
    QLabel* lbl_show_result{nullptr};
    ImageWidget* mImageWidget{nullptr};
    SensorMatch* mSensor_match{nullptr};
    QFuture<void> m_future;
    QFutureWatcher<void> m_watcher;
    match_data_t mMatch_data;
    QTimer mTimer;
    EnumType::match_type_e mMatch_type;
    int mCameraChannel{0};

    bool m_is_fusion_start{false};
    int m_timer_id{0};
    int m_timer_id_data_heck{0};
    const int m_timer_interval{70};

    bool mRunFlag{false};
    bool mPauseFlag{false};
    void start_fusion(match_data_t& match_data,cv::Mat& mat);
    void doWork();

signals:
    void sig_update_image(QImage img, int w, int h);
};

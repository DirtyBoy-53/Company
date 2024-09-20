#ifndef FUN_SWITCH_WIDGET_H
#define FUN_SWITCH_WIDGET_H

#include <QWidget>
#include "Media.h"
QT_FORWARD_DECLARE_CLASS(QCheckBox)
QT_FORWARD_DECLARE_CLASS(QPushButton)

class FunSwitchWidget : public QWidget
{
    Q_OBJECT
public:
    FunSwitchWidget(QWidget* parent = nullptr);
    ~FunSwitchWidget();
    void setMedia(Media &media);
    void initUI();
    void initConnect();
public slots:
    void on_checkBox_clicked(bool checked);
    void on_btn_clicked();
public:
    // 伪彩
    QCheckBox*      checkBox_white;
    QCheckBox*      checkBox_black;
    QCheckBox*      checkBox_rainbow;
    QCheckBox*      checkBox_green;
    QCheckBox*      checkBox_blue ;

    // 快门开关
    QPushButton*    btn_shutter_open;
    QPushButton*    btn_shutter_close;

    // 算法开关
    QPushButton*    btn_algorithm_open;
    QPushButton*    btn_algorithm_close;

    // 视频格式
    QPushButton*    btn_video_format_y8;
    QPushButton*    btn_video_format_y16;

    // NUC 补偿
    QPushButton*    btn_nuc_recoup;

    // 快门补偿
    QPushButton*    btn_shutter_recoup;



private:
    Media           mMedia;

};

#endif // FUN_SWITCH_WIDGET_H

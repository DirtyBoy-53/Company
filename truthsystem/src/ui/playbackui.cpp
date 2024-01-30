#include "playbackui.h"
#include "ui_playbackui.h"
#include <QFileDialog>
#include <QPixmap>
#include <QFile>
#include <QMessageBox>
#include <QDebug>
#include <QFileInfo>
#include <thread>
#include <nvs_result.h>
#include <QTime>
#include "yuv2rgb.h"
#include "util.h"
#include "memcache.h"


PlayBackUi::PlayBackUi(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::PlayBackUi)
{
    ui->setupUi(this);

    ui->pushButtonPre->setStyleSheet("border-image:url(:/image/zuozuo.png);");
    ui->pushButtonPlay->setStyleSheet("border-image:url(:/image/play.png);");
    ui->pushButtonNext->setStyleSheet("border-image:url(:/image/youyou.png);");
    ui->pushButtonStop->setStyleSheet("border-image:url(:/image/stop_play.png);");

    connect(&m_player,&PlayFactory::signalUpdatePlayData,this,&PlayBackUi::showVideo,Qt::DirectConnection);
    connect(ui->checkBoxNvsShow,&QCheckBox::stateChanged,this,&PlayBackUi::changeNvsSwitch);
    connect(ui->horizontalSlider,&QSlider::sliderReleased,this,&PlayBackUi::onSliderPressed);
    ui->horizontalSlider->setPageStep(0);
    ui->groupBoxPraser->setVisible(false);
}

PlayBackUi::~PlayBackUi()
{
    m_isRunning = false;
    if(m_thread != nullptr) {
        m_thread->join();
        delete m_thread;
        m_thread = nullptr;
    }
    delete ui;
}

void PlayBackUi::closeEvent(QCloseEvent *event)
{
    on_pushButtonStop_clicked();
}

void PlayBackUi::onSliderPressed()
{
    int index = ui->horizontalSlider->value();
    qDebug() << "slider value is " << index;
    m_player.moveTo(index);
}

void PlayBackUi::showVideo(QByteArray data, int curCount, int playStatus)
{
    qDebug() << "Data count:"<< curCount  << " status:"<< playStatus << " time:" << QDateTime::currentDateTime().toString("hh:mm:ss zzz");
    m_curCountIdx = curCount;
    ui->horizontalSlider->setValue(m_curCountIdx);
    QString sMsg = QString::number(m_curCountIdx) + "/" + QString::number(m_frameCount);
    ui->labelProcessShow->setText(sMsg);

    if(playStatus == RECORD_PALYING) {
        changePlayBtnState(1);
    } else if(playStatus == RECORD_SUPPEND) {
        changePlayBtnState(0);
    } else if(playStatus == RECORD_STOP) {
        changePlayBtnState(0);
    }

    if(!data.isNull()) {
        updateNvsResultText(data);
        ui->videoWidget->updateFrame(data);
    }
}

void PlayBackUi::on_pushButton_clicked()
{
    QString filePath = QFileDialog::getOpenFileName();
    if(filePath.isEmpty()) return;

    ui->lineEditFile->setText(filePath);
    // CollectInfo collectInfo;
    m_frameCount = m_player.openFile(filePath/*, collectInfo*/);
    ui->horizontalSlider->setMaximum(m_frameCount - 1);

    // QString text;
    // text.append(QString("视频帧率: %1 \n").arg(collectInfo.fps));
    // text.append(QString("采集场景: %1 \n").arg(g_sceneList[collectInfo.scene]));
    // text.append(QString("采集天气: %1 \n").arg(g_weatherList[collectInfo.weather]));
    // text.append(QString("采集类型: %1 \n").arg(g_typeList[collectInfo.type]));
    // text.append(QString("季节: %1 \n").arg(g_seasonList[collectInfo.season]));
    // text.append(QString("城市: %1 \n").arg(g_cityList[collectInfo.city]));
    // text.append(QString("温度: %1 \n").arg(collectInfo.temp));
    // text.append(QString("湿度: %1 \n").arg(collectInfo.wet));
    // ui->labelInfo->setText(text);
}

void PlayBackUi::on_pushButtonPlay_clicked()
{
    if(!m_bPlay) {
        QString sMsg = QString::number(m_curCountIdx) + "/" + QString::number(m_frameCount);
        ui->labelProcessShow->setText(sMsg);
        changePlayBtnState(STATUS_PLAY);
        m_player.play();
    }else {
        changePlayBtnState(STATUS_STOP);
        m_player.suspend();
    }
}


void PlayBackUi::changePlayBtnState(const int state)
{
    if(state) {
        m_bPlay = true;
        ui->pushButtonPlay->setStyleSheet("border-image:url(:/image/stop_play.png);");
    } else {
        m_bPlay = false;
        ui->pushButtonPlay->setStyleSheet("border-image:url(:/image/play.png);");
    }
}

void PlayBackUi::on_pushButtonStop_clicked()
{
    ui->horizontalSlider->setValue(0);
    m_curCountIdx = 0;
    changePlayBtnState(STATUS_STOP);
    m_player.stop();
}


void PlayBackUi::on_pushButtonPre_clicked()
{
    m_player.last();
}


void PlayBackUi::on_pushButtonNext_clicked()
{
    m_player.next();
}


void PlayBackUi::on_pushButton_3_clicked()
{
    getFrame(GET_FRAME_BMP);
}


void PlayBackUi::on_pushButton_4_clicked()
{
    getFrame(GET_FRAME_YUV);
}

void PlayBackUi::changeNvsSwitch(const int flag)
{
    MemCache::instance()->setValue("isRecordShowNvs", QString::number(flag));
    m_player.setNvsSwitch(flag);
}

void PlayBackUi::getFrame(const int mode)
{
    int num = ui->lineEditInteral->text().toInt();
    if(num <= 0 || num > 500) {
        QMessageBox::information(this, "提示", "抽帧间隔范围0-500");
        return;
    }

    QStringList filePathList = QFileDialog::getOpenFileNames();
    if(filePathList.size() <= 0) return;

    GetFrameConfig config;
    config.isGetNvsTxt = ui->checkBoxNvsTxt->isChecked();
    config.isGetParam = ui->checkBoxHasParam->isChecked();
    QString srcPath;
    YuvPlay yuvPlay;
    for(int i=0; i<filePathList.size(); i++) {
        srcPath = filePathList[i];
        yuvPlay.getFrame(srcPath, mode, num, config);
    }

    QMessageBox::information(this, "提示", "抽帧完成");
}

void PlayBackUi::updateNvsResultText(QByteArray data)
{
    if(!ui->checkBoxNvsShow->isChecked()) return;
    ui->editNvsResult->clear();

    FrameInfo_S* frameInfo = (FrameInfo_S*)data.data();
    if(frameInfo->uNvsLen <= 0) return;
    BoxMsg_t box = *(BoxMsg_t*)data.mid(128, sizeof(BoxMsg_t)).data();
    if(box.obj_number > 29) return;

    QString text;
    text.append(QString("obj_number: %1 \n").arg(box.obj_number));
    text.append(QString("is_alarm_frame: %1 \n").arg(box.is_alarm_frame&0xff));
    text.append(QString("car_fire_state: %1 \n").arg(box.car_fire_state));
    text.append(QString("car_speed: %1 \n").arg(box.car_speed));
    text.append(QString("obj_frame_id: %1 \n").arg(box.obj_frame_id));
    text.append(QString("obj_number: %1 \n").arg(box.obj_number));
    text.append(QString("camera_external.m_pitch: %1 \n").arg(box.camera_external.m_pitch));
    text.append(QString("camera_external.m_yaw: %1 \n").arg(box.camera_external.m_yaw));
    text.append(QString("camera_external.install_height: %1 \n").arg(box.camera_external.install_height));
    for(int i=0; i<box.obj_number; i++) {
        text.append(QString("obj_%1{ \n").arg(i));
        text.append(QString("    is_alarm: %1 \n").arg(box.bbox[i].is_alarm));
        text.append(QString("    class_id: %1 \n").arg(box.bbox[i].class_id));
        text.append(QString("    track_id: %1 \n").arg(box.bbox[i].track_id));
        text.append(QString("    ttc: %1 \n").arg(box.bbox[i].ttc));
        text.append(QString("    coordinates: (x1:%1,y1:%2) (x2:%3,y2:%4) \n").
                    arg(box.bbox[i].coordinates.x1).arg(box.bbox[i].coordinates.y1).
                    arg(box.bbox[i].coordinates.x2).arg(box.bbox[i].coordinates.y2));
        text.append(QString("    velocity: v1:%1 v2:%2\n").arg(box.bbox[i].velocity.v1).arg(box.bbox[i].velocity.v2));
        text.append(QString("    distance: x:%1 y:%2\n").arg(box.bbox[i].position.x).arg(box.bbox[i].position.y));

        text.append(QString("}obj_%1 \n").arg(i));
    }

    ui->editNvsResult->setText(text);
}

void PlayBackUi::on_btnGetVideoData_clicked()
{
    QString filePath = QFileDialog::getOpenFileName();
    if(filePath.isEmpty()) return;
    bool hasParam = ui->checkBoxHasParam->isChecked();
    m_player.getVideoData(filePath, hasParam);
    QMessageBox::information(this, "提示", "转换完成");
}



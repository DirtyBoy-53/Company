#include "cameradiswidget.h"
#include "ui_cameradiswidget.h"
#include "alg_sdk/client.h"
#include "alg_common/basic_types.h"
#include "memcache.h"
#include "alg_cvtColor.h"
#include "MyQOpenglWidget.h"
#include <iostream>
#include "MapNew.h"
#include "videopaintlabel.h"
#include <QCoreApplication>
#include <QDir>
#include <alg_sdk/service.h>
#include <common.h>
#include "ConStatus.hpp"
#include "datacollector.h"
SharedMemBuffer g_shared[PANE_NUM];

static uint8_t * g_cache[PANE_NUM];
static FrameInfo g_lasTm[PANE_NUM];

void callback_image_data(void *p);
void dataCall(uint8_t * data, int len, uint64_t tm);

cameradiswidget::cameradiswidget(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::cameradiswidget)
{
    qRegisterMetaType<StructPointData*>("StructPointData*");

    ui->setupUi(this);
    init();
  
    // qDebug() << "cameradiswidget::init " << QThread::currentThreadId();
    Log::Logger->info("cameradiswidget::init ->{}",QThread::currentThreadId());
    new std::thread(&cameradiswidget::startGet, this);
    memset (m_yuvPaint, 0, PANE_NUM);
}

cameradiswidget::~cameradiswidget()
{
    if(m_lidar_at128 != nullptr){
        delete m_lidar_at128;
        m_lidar_at128 = nullptr;
    }
    // if(m_viewer != nullptr){
    //  delete m_viewer;
    //  m_viewer = nullptr;           
    // }
    delete ui;
}

void cameradiswidget::init()
{
    for(int i = 0;i < PANE_NUM; i ++) {
        int row = i / 3;
        int col = i % 3;
        m_yuvPaint[i] = new GLYuvWidget(nullptr, i, true);
        ui->gridLayout->addWidget(m_yuvPaint[i], row, col);

        ui->gridLayout->setRowStretch(row,col);
        ui->gridLayout->setColumnStretch(row,col); 
    }
    ui->gridLayout->setSpacing(2);  
}

void cameradiswidget::slot_oneFrameProc(StructPointData* data){}
void cameradiswidget::slotShotOne(int id){}
void cameradiswidget::slot_lidar_view_dir(ENUM_VIEW view){}
void cameradiswidget::slot_set_lidar_path(QString &path){}
void cameradiswidget::slot_lidar_init(QString ip){}

#define R_LEN (20 * 1024 * 1024)
void cameradiswidget::showNewData(){}

/// @brief 
void cameradiswidget::startGet()
{
    int rc;
    char image_topic_names[ALG_SDK_MAX_CHANNEL][256];
    for (int i = 0; i < PANE_NUM; i++)
    {
        snprintf(image_topic_names[i], 256, "/image_data/stream/%02d", i);
        qInfo("subscribe %s",image_topic_names[i]);
        rc = alg_sdk_subscribe(image_topic_names[i], callback_image_data);
        if (rc < 0)
        {
            printf("Subscribe to topic %s Error!\n", image_topic_names[i]);
            exit(0);
        }
        g_cache[i] = (uint8_t *) malloc(20 * 1024 * 1024);
        g_shared[i].init((ESHARECHANNEL)i, 1);
    }
    if(alg_sdk_init_client())
    {
        printf("Init Client Error!\n");
        exit(0);
    }
    alg_sdk_client_spin_on();
}

// 回调 all
void callback_image_data(void *p)
{
    uint64_t tCur = QDateTime::currentMSecsSinceEpoch();
    pcie_image_data_t* msg = (pcie_image_data_t*)p;
    QString sName = QByteArray::fromRawData(msg->common_head.topic_name, strlen(msg->common_head.topic_name));

    if (sName.isEmpty()) {
        qDebug() << ">>> Empty Name";
        return;
    }
    int iIdx = sName.right(2).toInt();
    //qDebug() << ">>> Recieve Name: " << iIdx;
    // int channelId = MemCache::instance()->getI2CRowByChannel(iIdx);
    int channelId = iIdx;
    
    //qInfo() << "PCIE channel id "<< iIdx <<" 显示channel id "<< channelId;
    if(iIdx >= 0 && iIdx < PANE_NUM && MemCache::instance()->getChannel(channelId).typeId == TYPEI2CDIRECT) {
        uint64_t tLast = g_lasTm[channelId].tm;
        if(tLast + 10000 <= tCur) {
            g_lasTm[channelId].fps = g_lasTm[channelId].count;
            g_lasTm[channelId].count = 0;
            g_lasTm[channelId].tm = tCur;
            // qInfo()<< "channel "<< iIdx <<" fps "<<g_lasTm[channelId].fps/10.0;
        }
        g_lasTm[channelId].count++;
        FrameInfo_S head;
        // memset(head,0,sizeof(FrameInfo_S));
        head.uFormat = 1;// 1,y8 uyvy 3,y16
        head.uTotalLen = msg->image_info_meta.img_size + HEADSIZE;//大小
        head.width = msg->image_info_meta.width;
        head.height = msg->image_info_meta.height;
        head.uTimeTm = msg->image_info_meta.timestamp;
        head.ext[0] = msg->image_info_meta.frame_index;
        head.ext[1] = g_lasTm[channelId].fps;
        head.ext[2] = msg->image_info_meta.data_type;//37-ir 31-light 38-ir 1280
        head.uParamLen = 0;//初始化
        u_char* p = (u_char*)msg->payload;

        
        // qDebug("totalLen = %d, width = %d, height = %d, uTimeTm = %ld uTotalLen = %ld img_size = %ld ", 
        //             head.uTotalLen, head.width, head.height, head.uTimeTm, head.uTotalLen, msg->image_info_meta.img_size);
        if (head.ext[2] == 37 || head.ext[2] == 38) { // IR
            int ipos = 0;
            if (head.ext[2] == 37) {
                head.uParamLen = head.width * 2 * 4;//计算参数行长度
                head.height = msg->image_info_meta.height - 4;
                p = (u_char*)msg->payload;
                ipos = head.width * (head.height + 2) * 2; // height contains the param line (4 lines)
            } else {
                head.uTotalLen = msg->image_info_meta.img_size + HEADSIZE - head.width * 2 * 1;//throw first line...
                head.uParamLen = head.width * 2 * 1;
                head.height = msg->image_info_meta.height - 2;
                p = (u_char*)msg->payload + head.width * 2 * 1; // throw first line
                ipos = head.width * head.height * 2; // height contains the param line (4 lines)
            }

            if ((unsigned char)p[ipos] == 0x55 && (unsigned char)p[ipos + 1] == 0xAA) {
                head.uFormat = (unsigned char)(p[ipos + 17]);// 
                // qDebug() << "Current Data Type is " << head.uFormat << " 1-Y8, 3-Y16";
            }
            if (head.uFormat == 1) {
                head.uYuvLen = head.width * head.height * 2;
            } else if (head.uFormat == 3) {
                head.uY16Len = head.width * head.height * 2;
            }
        } else { // light
            head.uHeadFlag = 0xefefefef80808080;
            head.uYuvLen = 0;//head.width * head.height * 2;
            head.vlLen = head.width * head.height * 2;
            head.vlFormat = 4;//1:yuv420  4:yuv422
            head.uNvsLen = 0;
            head.uY16Len = 0;
            head.vlWidth = head.width;
            head.vlHeight = head.height;
            head.imuLen = 0;
        }
        // qDebug("After: totalLen = %d, width = %d, height = %d, uTimeTm = %ld", head.uTotalLen, head.width, head.height, head.uTimeTm);
        memcpy(g_cache[channelId], &head, HEADSIZE);
        
        memcpy(g_cache[channelId] + HEADSIZE, (char *)p, head.uTotalLen-HEADSIZE);
        // qDebug() << ">>> Before Shared Memory Write";
        int writeRet = g_shared[channelId].writeFrame((char *)g_cache[channelId],
                                  head.uTotalLen,
                                  msg->image_info_meta.timestamp);
        DataCollector::instance()->add((char*)g_cache[channelId],head.uTotalLen,msg->image_info_meta.timestamp,channelId);
        ChannelInfo ci = MemCache::instance()->getChannel(channelId);
        ci.proId = head.ext[2];//31 37 38
        MemCache::instance()->setValue(ci);

        #if 1
        emit GlobalSignals::getInstance()->signal_one_frame_light(channelId, g_cache[channelId], head.uTotalLen);
        // qDebug() << ">>> Write Shared Memory Return: " << writeRet;
        CConStatus::instance()->update_connect_time(channelId,QTime::currentTime());
        #endif
    }
    uint64_t time = QDateTime::currentMSecsSinceEpoch();
}

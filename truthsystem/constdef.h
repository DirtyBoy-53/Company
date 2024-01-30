#ifndef CONSTDEF_H
#define CONSTDEF_H

#include <QObject>
#include <functional>
#include <QMap>
#include <qglobal.h>

#define IR_VIDEO_ID 0
#define VL_VIDEO_ID 1

static const QString g_armAppVersion = "1.0.16";
static const int g_arrMaxLen = 1024*1024*4;
static const int g_frameMaxLen = 1024*1024;
static const int g_recordTime = 60;

static int g_collectHeadLen = 640 *2;

static const QString g_collectDir = "/collect/";
static const QString g_calibDir = "/calib_data/";

static const QList<QString> g_sceneList = {"隧道","眩光","施工路段","十字路口","高架桥高速路","栅栏、分隔带","商贩占道",
                                    "乡村道路","转弯路段","桥梁","上下匝道","公交站台","前车并道","行人鬼探头横穿",
                                    "路灯开启","学校上、放学","医院门口","小区里"};
static const QList<QString> g_typeList = {"步行人","骑行人","小车","特种车","人车混合","交通锥","路杆","动物","车道线"};
static const QList<QString> g_weatherList = {"晴天","小雨","中雨","大雨","雨后","薄雾","浓雾","小雪","大雪","雾霾"};
static const QList<QString> g_cityList = {"武汉","广州","天津","其他"};
static const QList<QString> g_seasonList = {"春","夏","秋","冬"};

enum GET_FRAME_MODE {
    GET_FRAME_BMP = 0,
    GET_FRAME_YUV
};







#endif // CONSTDEF_H

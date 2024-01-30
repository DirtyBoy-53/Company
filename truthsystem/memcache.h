#ifndef MEMCACHE_H
#define MEMCACHE_H
#include <QMap>
#include <QMutex>
#include "singleton.h"

#define MAXSIZECOUNT 9

#define TYPENONE 0
#define TYPEI2CDIRECT 1
#define TYPEJIGUANG 2
#define TYPEARTOSYN 3

struct ChannelInfo{
    int chId;//1 - 9
    int typeId = 0;//0 none 1 i2c 2 jiguang
    int tChanid = -1;//0 1 2 3
    int proId = 0; //0 none 1
};

class MemCache:public Singleton<MemCache>
{
public:
    MemCache();
    void setValue(QString key, QString value);
    QString value(QString key);

    void setValue(ChannelInfo info);
    int getI2CRowByChannel(int channelId);
    ChannelInfo getChannel(int id);

private:
    QMutex m_mutex;
    QMap<QString,QString> m_map;
    ChannelInfo m_channelInfo[MAXSIZECOUNT];
};

#endif // MEMCACHE_H

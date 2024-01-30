#include "memcache.h"

#define LOCKER QMutexLocker ll(&m_mutex);



MemCache::MemCache()
{

}

void MemCache::setValue(QString key, QString value)
{
    LOCKER;
    m_map[key] = value;
}

QString MemCache::value(QString key)
{
    LOCKER;
    if(m_map.find(key) != m_map.end()) {
        return m_map[key];
    }
    return "";
}

void MemCache::setValue(ChannelInfo info)
{
    LOCKER;
    m_channelInfo[info.chId] = info;
}

int MemCache::getI2CRowByChannel(int channelId)
{
    int idx = 0;
    LOCKER;
    for(int i = 0; i < MAXSIZECOUNT; i ++) {
        if(m_channelInfo[i].tChanid == channelId) {
            idx = i;
            break;
        }
    }

    return idx;
}

ChannelInfo MemCache::getChannel(int id)
{
    LOCKER;
    return m_channelInfo[id % MAXSIZECOUNT];
}

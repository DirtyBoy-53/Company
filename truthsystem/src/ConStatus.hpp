#ifndef CONSTATUS_HPP
#define CONSTATUS_HPP
#include <QTime>
#include "singleton.h"
#include "log.h"
#define channel_max_num     20
#define elapsed_max_num     5000    //5s未获取到数据
class CConStatus : public Singleton<CConStatus>
{
public:
    CConStatus(){
        for(auto i = 0;i < channel_max_num;++i){
            m_time[i] = QTime::fromString("00:00:00","hh:mm:ss");
        }
    }
private:
    QTime m_time[channel_max_num];

public:
    bool update_connect_time(int channel,QTime time){
        if(channel >= channel_max_num && channel >= 0){
            Log::Logger->warn("max channel num is {},but input channel num is {}",channel_max_num,channel);
            return false;
        }else{
            m_time[channel] = time;
            return true;
        }
    }
    bool get_connect_time(int channel,QTime &time){
        if(channel >= channel_max_num && channel >= 0){
            Log::Logger->warn("max channel num is {},but input channel num is {}",channel_max_num,channel);
            return false;
        }else{
            time = m_time[channel];
            return true;
        }
    }
};

#endif

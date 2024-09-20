#ifndef MEDIA_H
#define MEDIA_H

#include <string>
#include "YDefine.h"
#include <QString>

typedef struct media_s {
    media_type_e    type;
    std::string     src;
    // std::string     descr;
    int             id{-1};
    int             channel{-1}; // for camera index

    media_s() {
        type = MEDIA_TYPE_NONE;
        id = -1;
        channel = -1;
        src = "";
    }

    media_s(media_type_e type, std::string src, int id, int channel) {
        this->type = type;
        this->src = src;
        this->id = id;
        this->channel = channel;
    }
    int group() const {
        return channel/2;
    }
    QString toString(){
        return QString("type:%1, src:%2 id:%3 ch:%4 group:%5")
            .arg(type).arg(QString::fromStdString(src)).arg(id).arg(channel).arg(group());
    }
    media_s &operator = (const media_s &media){
        if(this == &media) return *this;
        this->channel   = media.channel;
        this->src       = media.src;
        this->id        = media.id;
        this->type      = media.type;
    }
} Media;

#endif // MEDIA_H

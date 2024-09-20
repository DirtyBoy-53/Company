#ifndef VIDEOPLAYERFACTORY_H
#define VIDEOPLAYERFACTORY_H

#include "Media.h"
#include "VideoPlayer.h"
#include "XYZJVideo.h"

enum renderer_type_e {
    RENDERER_TYPE_OPENGL,
    RENDERER_TYPE_QPAINTER,
};
#define DEFAULT_RENDERER_TYPE RENDERER_TYPE_OPENGL
class VideoPlayerFactory
{
public:
    static VideoPlayer* create(media_type_e type) {
        switch (type) {
        case MEDIA_TYPE_FILE:
//            return new XXXVideo;
        case MEDIA_TYPE_PCIE:
            return new XYZJVideo;

        default:
            return NULL;
        }
    }
};

#endif // VIDEOPLAYERFACTORY_H

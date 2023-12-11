#ifndef CUSTOM_EVENT_TYPE_H
#define CUSTOM_EVENT_TYPE_H

class QCustomEvent {
public:
    enum Type{
        User = 10000,
        OpenMediaSucceed,
        OpenMediaFailed,
        PlayerEOF,
        PlayerError,
    };
};

#endif // CUSTOM_EVENT_TYPE_H

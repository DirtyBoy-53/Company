#ifndef CUSTOMEVENT_H
#define CUSTOMEVENT_H

class CustomEvent {
public:
    enum Type{
        User = 10000,
        OpenMediaSucceed,
        OpenMediaFailed,
        PlayerError,
    };
};

#endif // CUSTOMEVENT_H

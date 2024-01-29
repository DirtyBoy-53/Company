#ifndef ELECLOCK_PARAM_H
#define ELECLOCK_PARAM_H
#include <QString>

struct AutoWorkInfo {
    int workIndex = 0;
    int workState = 0;
    int result;
    int errorCode;
    float circle;
    float maxMnm;
    int workTime;
    int reworkTimes = 1;
    int direction;
};

struct AutoPosState {
    int x;
    int y;
    bool isCrc;
    bool enable = false;
    int offset;
    QString imgPath;
    QString name;
};

struct AutoWorkResult {
    QString name;
    int index;
    QString workPos;
    float workMnm;
    float workCircle;
    int workTime;
    int reworkTimes;
    int errorCode;
    int result;
};

#endif // ELECLOCK_PARAM_H

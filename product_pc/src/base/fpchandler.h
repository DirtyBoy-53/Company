#ifndef FPCHANDLER_H
#define FPCHANDLER_H

#include <QObject>
#include <QTcpServer>
#include "csyncsocket.h"

enum PlcMoveState {
    PLC_MOVE_SHUTTER_CHECK = 1,
    PLC_MOVE_FPC_CHECK = 2,
};

struct FpcCheckResult {
    bool result = false;
    float mm;
    float jdc;
};

class FpcHandler : public QObject
{
    Q_OBJECT
public:
    explicit FpcHandler();
    ~FpcHandler();

    bool initPlcWrite(QString ip, int port);
    bool initPlcRead(QString ip, int port);
    bool initCamera(QString ip, int port);
    bool deInit();

    bool plcReadyOk();
    int plcWaitStart();
    bool plcMoveTo(PlcMoveState state);
    bool plcCheck();
    bool plcReset();
    bool plcAutoMode();
    bool plcSetResult(const bool result);
    bool plcWaitOrigin();
    bool plcClearRead();

    FpcCheckResult cameraCheck();


private:
    bool plcCheckError(int checkByte);
    CSyncSocket* m_plcWrite;
    CSyncSocket* m_plcRead;
    CSyncSocket* m_camera;

};

#endif // FPCHANDLER_H

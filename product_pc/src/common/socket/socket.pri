INCLUDEPATH += $$PWD
DEPENDPATH += $$PWD

INCLUDEPATH += $$PWD/../../../include/poco
LIBS += -L$$PWD/../../../lib/poco -lPocoZip
LIBS += -L$$PWD/../../../lib/poco -lPocoNet
LIBS += -L$$PWD/../../../lib/poco -lPocoJSON
LIBS += -L$$PWD/../../../lib/poco -lPocoUtil

HEADERS +=  \
    $$PWD/CPingTest.h \
    $$PWD/HttpClient.h \
    $$PWD/csyncsocket.h \
    $$PWD/ctcpsocket.h \
    $$PWD/jsonmanager.h \
    $$PWD/mesclient.h \
    $$PWD/mescom.h \
    $$PWD/messagedispacher.h \
    $$PWD/messageid.h \
    $$PWD/socketmanager.h \
    $$PWD/cwebsocket.h \
    $$PWD/WebSocketClientManager.h \
    $$PWD/tcpclient.h \
    $$PWD/videoinfo.h \
    $$PWD/websocketclient.h

SOURCES += \
    $$PWD/CPingTest.cpp \
    $$PWD/HttpClient.cpp \
    $$PWD/csyncsocket.cpp \
    $$PWD/ctcpsocket.cpp \
    $$PWD/jsonmanager.cpp \
    $$PWD/mesclient.cpp \
    $$PWD/mescom.cpp \
    $$PWD/messagedispacher.cpp \
    $$PWD/socketmanager.cpp \
    $$PWD/cwebsocket.cpp \
    $$PWD/WebSocketClientManager.cpp \
    $$PWD/tcpclient.cpp \
    $$PWD/videoinfo.cpp \
    $$PWD/websocketclient.cpp



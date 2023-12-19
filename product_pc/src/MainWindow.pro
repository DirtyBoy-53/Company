#-------------------------------------------------
#
# Project created by QtCreator 2022-11-24T09:13:33
#
#-------------------------------------------------

QT       += core gui xml network serialport websockets

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets
CONFIG += C++11
DESTDIR = $$PWD/../out
TARGET = AutoProduce
TEMPLATE = app

DEFINES += QT_DEPRECATED_WARNINGS

include (common/common.pri)
include (ui/ui.pri)
include (base/base.pri)
include (res/res.pri)
include (produce/produce.pri)

INCLUDEPATH += \
    $$PWD/../include \
    $$PWD/../include/ffmpeg \
    $$PWD/../include/zlgcan \

CONFIG(debug, debug|release){
    DEFINES += QT_MESSAGELOGCONTEXT
    QMAKE_CXXFLAGS_RELEASE = $$QMAKE_CFLAGS_RELEASE_WITH_DEBUGINFO
    QMAKE_LFLAGS_RELEASE = $$QMAKE_LFLAGS_RELEASE_WITH_DEBUGINFO
    LIBS += -L$$PWD/../lib/calibSdk/debug -lIRCalibIntrinsic
}else {
    LIBS += -L$$PWD/../lib/calibSdk/release -lIRCalibIntrinsic
    QMAKE_CXXFLAGS_RELEASE = $$QMAKE_CFLAGS_RELEASE_WITH_DEBUGINFO
    QMAKE_LFLAGS_RELEASE = $$QMAKE_LFLAGS_RELEASE_WITH_DEBUGINFO
    QMAKE_CXXFLAGS_RELEASE_WITH_DEBUGINFO += -Od
}


DISTFILES += \
    ../out/plugin.xml

HEADERS +=

SOURCES +=

RC_ICONS = $$PWD/image/logo1.ico

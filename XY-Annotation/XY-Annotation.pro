QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += c++17
DEFINES -= UNICODE _UNICODE

TEMPLATE = app
#RC_ICONS = app.ico
#RC_FILE = app.rc

TARGET          = XY-Annotation
MOC_DIR         = build/moc
RCC_DIR         = build/rcc
UI_DIR          = build/ui
OBJECTS_DIR     = build/obj

DEFINES += QT_DEPRECATED_WARNINGS
RESOURCES += res/skin/dark/darkstyle.qrc res/skin/light/lightstyle.qrc
#TRANSLATIONS = rc/lang/app_zh_CN.ts rc/lang/app_zh_CN.qm

INCLUDEPATH += src

#global
SRC_GLOBAL = src/global
INCLUDEPATH += $$SRC_GLOBAL
SOURCES +=\
    $$SRC_GLOBAL/main.cpp
HEADERS += \
    $$SRC_GLOBAL/confile.h \
    $$SRC_GLOBAL/appdef.h \

#qt
SRC_QT = src/qt
INCLUDEPATH += $$SRC_QT
SOURCES += \
    $$SRC_QT/glwidget.cpp
HEADERS += \
    $$SRC_QT/custom_event_type.h \
    $$SRC_QT/glwidget.h \
    $$SRC_QT/qtfunctions.h \
    $$SRC_QT/qtheaders.h

#hv
DEFINES += HV_SOURCE
SRC_HV = src/hv
SRC_HV_BASE = src/hv/base
SRC_HV_UTILS = src/hv/utils
HEADERS += \
    $$SRC_HV/hv.h \
    $$SRC_HV/hconfig.h \
    $$SRC_HV/hexport.h \
    $$SRC_HV_BASE/hplatform.h \
    $$SRC_HV_BASE/hdef.h \
    $$SRC_HV_BASE/hbase.h \
    $$SRC_HV_BASE/hversion.h \
    $$SRC_HV_BASE/htime.h \
    $$SRC_HV_BASE/herr.h \
    $$SRC_HV_BASE/hbuf.h \
    $$SRC_HV_BASE/hlog.h \
    $$SRC_HV_BASE/hscope.h \
    $$SRC_HV_BASE/hstring.h \
    $$SRC_HV_BASE/hmutex.h \
    $$SRC_HV_BASE/hthread.h \
    $$SRC_HV_UTILS/singleton.h \
    $$SRC_HV_UTILS/iniparser.h

SOURCES += \
    src/hv/base/hversion.c \
    src/hv/base/hbase.c \
    src/hv/base/herr.c \
    src/hv/base/htime.c \
    src/hv/base/hlog.c \
    src/hv/base/hstring.cpp \
    src/hv/utils/iniparser.cpp

win32{
#添加库
#    LIBS +=

#指定输出路径
    win32-msvc {
        if (contains(DEFINES, WIN64)) {
            DESTDIR = $$_PRO_FILE_PWD_/bin/msvc2015_x64
#            LIBS += -L$$_PRO_FILE_PWD_/3rd/lib/msvc2015_x64
        } else {
            DESTDIR = $$_PRO_FILE_PWD_/bin/msvc2015_x86
#            LIBS += -L$$_PRO_FILE_PWD_/3rd/lib/msvc2015_x86
        }
    }

    win32-g++ {
        QMAKE_CFLAGS += -std=c99
        QMAKE_CXXFLAGS += -std=c++11
        if (contains(DEFINES, WIN64)) {
            DESTDIR = $$_PRO_FILE_PWD_/bin/mingw64
#            LIBS += -L$$_PRO_FILE_PWD_/3rd/lib/mingw64
        } else {
            DESTDIR = $$_PRO_FILE_PWD_/bin/mingw32
#            LIBS += -L$$_PRO_FILE_PWD_/3rd/lib/mingw32
        }

        # for ffmpeg staticlib
        LIBS += -liconv \
        -lz     \
        -lbz2   \
        -llzma  \
        -lcrypto \
        -lbcrypt
    }
}

QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += c++17

CONFIG += sdk_no_version_check
DEFINES -= UNICODE _UNICODE
msvc {
    QMAKE_CFLAGS += /utf-8
    QMAKE_CXXFLAGS += /utf-8
}
TEMPLATE = app
#RC_ICONS = app.ico
#RC_FILE = app.rc

TARGET          = XY-Annotation
MOC_DIR         = build/moc
RCC_DIR         = build/rcc
UI_DIR          = build/ui
OBJECTS_DIR     = build/obj

DEFINES += QT_DEPRECATED_WARNINGS
RESOURCES += \
    res/skin/dark/darkstyle.qrc \
    res/skin/light/lightstyle.qrc \
    res/image/image.qrc
#TRANSLATIONS = rc/lang/app_zh_CN.ts rc/lang/app_zh_CN.qm

INCLUDEPATH += src

#global
SRC_GLOBAL = src/global
INCLUDEPATH += $$SRC_GLOBAL
SOURCES +=\
    $$SRC_GLOBAL/main.cpp \
    $$SRC_GLOBAL/common.cpp \
    src/annotations/shape.cpp \
    src/annotations/ypolygon.cpp \
    src/annotations/yrectangle.cpp \
    src/canvas/document2d.cpp \
    src/canvas/documentbase.cpp \
    src/controllers/commands.cpp \
    src/ui/canvaswidget.cpp \
    src/ui/labeldialog.cpp \
    src/ui/window.cpp



HEADERS += \
    $$SRC_GLOBAL/confile.h \
    $$SRC_GLOBAL/appdef.h \
    $$SRC_GLOBAL/common.h \
    src/annotations/shape.h \
    src/annotations/shapefactory.h \
    src/annotations/ypolygon.h \
    src/annotations/yrectangle.h \
    src/canvas/document2d.h \
    src/canvas/documentbase.h \
    src/controllers/commands.h \
    src/ui/canvaswidget.h \
    src/ui/labeldialog.h \
    src/ui/window.h



#utils
SRC_UTILS = src/utils
INCLUDEPATH += $$SRC_UTILS
#SOURCES +=\
#    $$SRC_UTILS/
#HEADERS += \
#    $$SRC_UTILS/

#qt
SRC_QT = src/qt
INCLUDEPATH += $$SRC_QT
#SOURCES += \

HEADERS += \
    $$SRC_QT/custom_event_type.h \
    $$SRC_QT/qtfunctions.h \
    $$SRC_QT/qtheaders.h \
    $$SRC_QT/qtstyles.h

#ui
SRC_UI = src/ui
INCLUDEPATH += $$SRC_UI
SOURCES += \
    $$SRC_UI/mainwindow.cpp \
    $$SRC_UI/customlistwidget.cpp
HEADERS += \
    $$SRC_UI/mainwindow.h \
    $$SRC_UI/customlistwidget.h


#annotations
SRC_ANO = src/annotations
INCLUDEPATH += $$SRC_ANO
SOURCES += \
    $$SRC_ANO/annotationitem.cpp \
    $$SRC_ANO/segannotationitem.cpp
HEADERS += \
    $$SRC_ANO/annotationitem.h \
    $$SRC_ANO/segannotationitem.h

#canvas
SRC_CANVAS = src/canvas
INCLUDEPATH += $$SRC_CANVAS
SOURCES +=\
    $$SRC_CANVAS/canvas2d.cpp \
    $$SRC_CANVAS/canvasbase.cpp
HEADERS +=\
    $$SRC_CANVAS/canvas2d.h \
    $$SRC_CANVAS/canvasbase.h

#controllers
SRC_CONTROL = src/controllers
INCLUDEPATH += $$SRC_CONTROL
SOURCES +=\
    $$SRC_CONTROL/annotationcontainer.cpp \
    $$SRC_CONTROL/filemanager.cpp \
    $$SRC_CONTROL/labelmanager.cpp
HEADERS +=\
    $$SRC_CONTROL/annotationcontainer.h \
    $$SRC_CONTROL/filemanager.h \
    $$SRC_CONTROL/labelmanager.h \


#3rd
INCLUDEPATH += 3rd

#spdlog
_3RD_SPDLOG = 3rd/spdlog
INCLUDEPATH += $$_3RD_SPDLOG
SOURCES += \
    $$_3RD_SPDLOG/ylog.cpp
HEADERS += \
    $$_3RD_SPDLOG/ylog.h


#hv
DEFINES += HV_SOURCE
SRC_HV = src/hv
SRC_HV_BASE = src/hv/base
SRC_HV_UTILS = src/hv/utils
INCLUDEPATH += $$SRC_HV
INCLUDEPATH += $$SRC_HV_BASE
INCLUDEPATH += $$SRC_HV_UTILS
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
    $$SRC_HV_BASE/hversion.c \
    $$SRC_HV_BASE/hbase.c \
    $$SRC_HV_BASE/herr.c \
    $$SRC_HV_BASE/htime.c \
    $$SRC_HV_BASE/hlog.c \
    $$SRC_HV_BASE/hstring.cpp \
    $$SRC_HV_UTILS/iniparser.cpp

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
    }
}

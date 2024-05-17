QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

#CONFIG += c++17
QMAKE_CXXFLAGS += -std=C++17

CONFIG += sdk_no_version_check
DEFINES -= UNICODE _UNICODE
#msvc {
#    QMAKE_CFLAGS += /utf-8
#    QMAKE_CXXFLAGS += /utf-8
#}
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

include(3rd/Thirdparty.pri)
include(src/Src.pri)

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

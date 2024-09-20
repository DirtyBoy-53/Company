QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

#定义编译选项
#QT_DEPRECATED_WARNINGS表示当Qt的某些功能被标记为过时的,那么编译器会发出警告
DEFINES += QT_DEPRECATED_WARNINGS

#指定编译器选项和项目配置
CONFIG  += c++14
CONFIG  += warn_on           #告诉qmake要把编译器设置为输出警告信息的
CONFIG  += precompile_header #可以在项目中使用预编译头文件的支持


TEMPLATE = app

TARGET          = XYPlayer
MOC_DIR         = Temp/moc
RCC_DIR         = Temp/rcc
UI_DIR          = Temp/ui
OBJECTS_DIR     = Temp/obj

include($$PWD/../3rd/Thirdparty.pri)
include($$PWD/Global/Global.pri)
include($$PWD/Window/Window.pri)
include($$PWD/Util/Util.pri)
include($$PWD/Video/Video.pri)
include($$PWD/Protocol/Protocol.pri)

win32{
    msvc {
        QMAKE_CFLAGS += /utf-8
        QMAKE_CXXFLAGS += /utf-8
    }
#指定输出路径
    win32-msvc {
        if (contains(DEFINES, WIN64)) {
            DESTDIR = $$_PRO_FILE_PWD_/../bin/msvc2015_x64
        } else {
            DESTDIR = $$_PRO_FILE_PWD_/../bin/msvc2015_x86
        }
    }

    win32-g++ {
        QMAKE_CFLAGS += -std=c99
        QMAKE_CXXFLAGS += -std=c++11
        if (contains(DEFINES, WIN64)) {
            DESTDIR = $$_PRO_FILE_PWD_/../bin/mingw64
        } else {
            DESTDIR = $$_PRO_FILE_PWD_/../bin/mingw32
        }
    }
}

unix{
    LIBS += \
            -lGLU   \
            -lGL    \
            -lm     \
            -lGLEW  \
            -lrt    \


    DESTDIR = $$_PRO_FILE_PWD_/../bin/Ubuntu20.04
    
    QMAKE_CXXFLAGS += -fopenmp
    QMAKE_LFLAGS += -fopenmp
}
RESOURCES += \
    ../Res/file.qrc \
    ../Res/img.qrc

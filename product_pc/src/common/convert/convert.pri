INCLUDEPATH += $$PWD
DEPENDPATH += $$PWD

INCLUDEPATH += $$PWD/../../../include

CONFIG += debug_and_release
CONFIG(release, release | debug){
    LIBS += -L$$PWD/../../../lib/opencv -lopencv_world330
}else{
    LIBS += -L$$PWD/../../../lib/opencv -lopencv_world330d
}
HEADERS += \
    $$PWD/qttoopencv.h \
    $$PWD/videoformatparser.h \
    $$PWD/yuv2rgb.h

SOURCES += \
    $$PWD/qttoopencv.cpp \
    $$PWD/videoformatparser.cpp \
    $$PWD/yuv2rgb.cpp



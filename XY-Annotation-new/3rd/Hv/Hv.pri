INCLUDEPATH += $$PWD
DEPENDPATH += $$PWD

DEFINES += HV_SOURCE
SRC_HV       = $$PWD/
SRC_HV_BASE  = $$PWD/base
SRC_HV_UTILS = $$PWD/utils
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


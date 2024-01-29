QT += axcontainer printsupport
INCLUDEPATH += $$PWD
HEADERS += \
    $$PWD/CLeakageRelative.h \
    $$PWD/LeakageDE1006.h \
    $$PWD/NonUniformDE1008.h \
    $$PWD/autocalibinde1010.h \
    $$PWD/autofpccheck.h \
    $$PWD/autoprocesscheckArm.h \
    $$PWD/autoprocesscheckSensor.h \
    $$PWD/autoprocesseleclock.h \
    $$PWD/autoprocesseleclockCB.h \
    $$PWD/autoprocessguide.h \
    $$PWD/baseproduce.h \
    $$PWD/burningarmce1001.h \
    $$PWD/burningde1000.h \
    $$PWD/burningmcucb1000.h \
    $$PWD/comproduce.h \
    $$PWD/dispensingcuring.h \
    $$PWD/dustimagedetection.h \
    $$PWD/eleclock_param.h \
    $$PWD/httppassstation.h \
    $$PWD/mtfcl1004.h \
    $$PWD/productpacking.h \
    $$PWD/armcb1002.h

SOURCES += \
    $$PWD/LeakageDE1006.cpp \
    $$PWD/NonUniformDE1008.cpp \
    $$PWD/autocalibinde1010.cpp \
    $$PWD/autofpccheck.cpp \
    $$PWD/autoprocesscheckArm.cpp \
    $$PWD/autoprocesscheckSensor.cpp \
    $$PWD/autoprocesseleclock.cpp \
    $$PWD/autoprocesseleclockCB.cpp \
    $$PWD/autoprocessguide.cpp \
    $$PWD/baseproduce.cpp \
    $$PWD/burningarmce1001.cpp \
    $$PWD/burningde1000.cpp \
    $$PWD/burningmcucb1000.cpp \
    $$PWD/comproduce.cpp \
    $$PWD/dispensingcuring.cpp \
    $$PWD/dustimagedetection.cpp \
    $$PWD/httppassstation.cpp \
    $$PWD/mtfcl1004.cpp \
    $$PWD/productpacking.cpp \
    $$PWD/armcb1002.cpp

LIBS += -lws2_32



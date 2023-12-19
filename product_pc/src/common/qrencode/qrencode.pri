INCLUDEPATH += $$PWD
DEPENDPATH += $$PWD

HEADERS += \
    $$PWD/bitstream.h \
    $$PWD/config.h \
    $$PWD/libqrencode.h \
    $$PWD/mask.h \
    $$PWD/mmask.h \
    $$PWD/mqrspec.h \
    $$PWD/qrencode.h \
    $$PWD/qrencode_inner.h \
    $$PWD/qrinput.h \
    $$PWD/qrspec.h \
    $$PWD/rsecc.h \
    $$PWD/split.h

SOURCES += \
    $$PWD/libqrencode.cpp \
    $$PWD/bitstream.c \
    $$PWD/mask.c \
    $$PWD/mmask.c \
    $$PWD/mqrspec.c \
    $$PWD/qrencode.c \
    $$PWD/qrinput.c \
    $$PWD/qrspec.c \
    $$PWD/rsecc.c \
    $$PWD/split.c





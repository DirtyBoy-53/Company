INCLUDEPATH += $$PWD
DEPENDPATH  += $$PWD

INCLUDEPATH += \
        $$PWD/SDK/  \
        $$PWD/SDK/include   \
        $$PWD/SDK/include/alg_common    \
        $$PWD/SDK/include/alg_sdk       \
        $$PWD/SDK/include/gl_display    \
        $$PWD/SDK/include/jetson-utils
DEPENDPATH  += \
        $$PWD/SDK/  \
        $$PWD/SDK/include   \
        $$PWD/SDK/include/alg_common    \
        $$PWD/SDK/include/alg_sdk       \
        $$PWD/SDK/include/gl_display    \
        $$PWD/SDK/include/jetson-utils

win32:mingw{
    INCLUDEPATH += $$PWD/SDK/lib/mingw32

    LIBS += -L$$PWD/SDK/lib/mingw32/ -lpcie_sdk \
            -L$$PWD/SDK/lib/mingw32/ -lpcie_sdk_cuda \
            -L$$PWD/SDK/lib/mingw32/ -lpcie_sdk_stream \
            -L$$PWD/SDK/lib/mingw32/ -lwinpthread-1
}

unix{
    INCLUDEPATH += $$PWD/SDK/lib/linux/x86_64
    LIBS += -L$$PWD/SDK/lib/linux/x86_64 -lpcie_sdk \
            -L$$PWD/SDK/lib/linux/x86_64 -lpcie_sdk_cuda \
            -L$$PWD/SDK/lib/linux/x86_64 -lpcie_sdk_stream
}

HEADERS += \
    $$PWD/AlgSdkManager.h

SOURCES += \
    $$PWD/AlgSdkManager.cpp
